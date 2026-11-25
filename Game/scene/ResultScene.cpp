#include "stdafx.h"
#include "scene/ResultScene.h"

#include "input/InputManager.h"
#include "param/GameParam.h"
#include "scene/Fade.h"
#include "scene/TitleScene.h"
#include "sound/GameSound.h"

namespace
{
	// Vector4 は constexpr にできないので、クラスの定数ではなくここに置く
	const Vector4 GRID_COLOR(1.0f, 0.17f, 0.84f, 1.0f);		///< 背景の格子のマゼンタ（タイトルと見分ける）
	const Vector4 HEADER_COLOR(1.0f, 0.17f, 0.84f, 1.0f);	///< 「RESULT」のマゼンタ
	const Vector4 SUB_COLOR(0.0f, 0.9f, 1.0f, 1.0f);		///< 「FIREWALL BREACHED」のシアン
	const Vector4 LABEL_COLOR(0.0f, 0.9f, 1.0f, 1.0f);		///< 見出しのシアン（HUD と同じ）
	const Vector4 VALUE_COLOR(1.0f, 1.0f, 1.0f, 1.0f);		///< 値の白
	const Vector4 GOLD_COLOR(1.0f, 0.83f, 0.0f, 1.0f);		///< ランクの枠と「RANK」の金（#ffd400）
	const Vector4 PRESS_COLOR(1.0f, 1.0f, 1.0f, 1.0f);		///< 「PRESS [A] TITLE」の白

	/** ランクの文字の色（S は金、A はシアン、B は白、C は灰色） */
	Vector4 RankColor(const char rank)
	{
		switch (rank)
		{
		case 'S':	return Vector4(1.0f, 0.9f, 0.3f, 1.0f);
		case 'A':	return Vector4(0.0f, 0.9f, 1.0f, 1.0f);
		case 'B':	return Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		default:	return Vector4(0.6f, 0.65f, 0.7f, 1.0f);
		}
	}

	/** text の長さで HudText を作って、文字と色を入れる */
	void InitText(HudText& hudText, const char* text, const float height, const HudText::Align align, const Vector4& color)
	{
		hudText.Init(static_cast<int>(strlen(text)), height, align);
		hudText.SetText(text);
		hudText.SetColor(color);
	}
}

void ResultScene::SetResult(const float clearTime, const float averageLevel, const int defeatCount)
{
	m_clearTime = clearTime;
	m_averageLevel = averageLevel;
	m_defeatCount = defeatCount;
}

char ResultScene::CalcRank() const
{
	const ResultParam& param = nsGameParam::Get().result;

	// タイムの点: timeBest 以下で 1、timeWorst 以上で 0、その間はまっすぐ
	const float timeRange = max(param.timeWorst - param.timeBest, 0.001f);
	const float timePoint = max(0.0f, min((param.timeWorst - m_clearTime) / timeRange, 1.0f));
	// 速度 Lv の点: Lv1 で 0、Lv5 で 1
	const float levelPoint = max(0.0f, min((m_averageLevel - MIN_LEVEL) / (MAX_LEVEL - MIN_LEVEL), 1.0f));
	const float score = timePoint * param.timeWeight + levelPoint * (1.0f - param.timeWeight);

	if (score >= param.rankS) { return 'S'; }
	if (score >= param.rankA) { return 'A'; }
	if (score >= param.rankB) { return 'B'; }
	return 'C';
}

bool ResultScene::Start()
{
	m_fade = FindGO<Fade>(Fade::NAME);
	if (m_fade != nullptr)
	{
		m_fade->StartFadeIn(FADE_TIME);
	}

	m_rank = CalcRank();

	m_back.Init("Assets/shader/sceneBack.fx", UI_SPACE_WIDTH, UI_SPACE_HEIGHT);
	m_rankBox.Init("Assets/shader/resultRank.fx", RANK_BOX_WIDTH, RANK_BOX_HEIGHT);

	// HudText は 1 つに 32 文字まで
	InitText(m_header, "RESULT", HEADER_HEIGHT, HudText::Align::Left, HEADER_COLOR);
	InitText(m_sub, "FIREWALL BREACHED", SUB_HEIGHT, HudText::Align::Left, SUB_COLOR);
	InitText(m_timeLabel, "TIME", LABEL_HEIGHT, HudText::Align::Left, LABEL_COLOR);
	InitText(m_levelLabel, "AVG SPEED LV", LABEL_HEIGHT, HudText::Align::Left, LABEL_COLOR);
	InitText(m_defeatLabel, "DEFEATS", LABEL_HEIGHT, HudText::Align::Left, LABEL_COLOR);
	InitText(m_rankLabel, "RANK", RANK_LABEL_HEIGHT, HudText::Align::Center, GOLD_COLOR);
	InitText(m_press, "PRESS [A] TITLE", PRESS_HEIGHT, HudText::Align::Center, PRESS_COLOR);

	const int minutes = static_cast<int>(m_clearTime / SECONDS_PER_MINUTE);
	const float seconds = m_clearTime - minutes * SECONDS_PER_MINUTE;
	char timeText[32];
	sprintf_s(timeText, "%02d:%05.2f", minutes, seconds);
	InitText(m_timeValue, timeText, VALUE_HEIGHT, HudText::Align::Right, VALUE_COLOR);

	char levelText[32];
	sprintf_s(levelText, "LV %.1f", m_averageLevel);
	InitText(m_levelValue, levelText, VALUE_HEIGHT, HudText::Align::Right, VALUE_COLOR);

	char defeatText[16];
	sprintf_s(defeatText, "%d", m_defeatCount);
	InitText(m_defeatValue, defeatText, VALUE_HEIGHT, HudText::Align::Right, VALUE_COLOR);

	const char rankText[2] = { m_rank, '\0' };
	InitText(m_rankLetter, rankText, RANK_LETTER_HEIGHT, HudText::Align::Center, RankColor(m_rank));

	return true;
}

void ResultScene::Update()
{
	InputManager& input = InputManager::GetInstance();
	input.Update();
	m_time += g_gameTime->GetFrameDeltaTime();

	// ランクを押した瞬間に音を鳴らす（押したことが耳でも分かるように）
	if (!m_isStamped && m_time >= RANK_TIME + STAMP_TIME)
	{
		m_isStamped = true;
		nsSound::Play(EnSound::Defeat);
	}

	const bool canInput = m_time >= INPUT_TIME;
	if (!m_isLeaving && canInput && (input.IsTrigger(EnInputAction::Jump) || input.IsTrigger(EnInputAction::Pause)))
	{
		m_isLeaving = true;
		if (m_fade != nullptr)
		{
			m_fade->StartFadeOut(FADE_TIME);
		}
	}

	if (m_isLeaving && (m_fade == nullptr || m_fade->IsBlack()))
	{
		NewGO<TitleScene>(0, "title");
		DeleteGO(this);
	}
}

void ResultScene::DrawRow(HudText& label, HudText& value, const float y, const float appearTime)
{
	if (m_time < appearTime)
	{
		return;
	}
	label.SetPosition(Vector2(LEFT_X, y + LABEL_HEIGHT));
	value.SetPosition(Vector2(VALUE_RIGHT_X, y - LABEL_HEIGHT * 0.5f));
	label.Draw();
	value.Draw();
}

void ResultScene::Render(RenderContext& rc)
{
	m_back.Update(Vector2::Zero, GRID_COLOR, GRID_SPEED);
	m_back.Draw();

	m_header.SetPosition(Vector2(LEFT_X, HEADER_Y));
	m_header.Draw();
	m_sub.SetPosition(Vector2(LEFT_X, SUB_Y));
	m_sub.Draw();

	DrawRow(m_timeLabel, m_timeValue, ROW_TOP_Y, FIRST_ROW_TIME);
	DrawRow(m_levelLabel, m_levelValue, ROW_TOP_Y - ROW_SPACING, FIRST_ROW_TIME + ROW_INTERVAL);
	DrawRow(m_defeatLabel, m_defeatValue, ROW_TOP_Y - ROW_SPACING * 2.0f, FIRST_ROW_TIME + ROW_INTERVAL * 2.0f);

	// ランク: 大きいところから縮んで押され、押した瞬間に枠が白く光る
	if (m_time >= RANK_TIME)
	{
		const float stampRate = min((m_time - RANK_TIME) / STAMP_TIME, 1.0f);
		const float scale = STAMP_SCALE + (1.0f - STAMP_SCALE) * stampRate;
		const float flash = (stampRate >= 1.0f) ? max(0.0f, 1.0f - (m_time - RANK_TIME - STAMP_TIME) / FLASH_TIME) : 0.0f;

		m_rankBox.Update(Vector2(RANK_X, RANK_Y), Vector4(GOLD_COLOR.x, GOLD_COLOR.y, GOLD_COLOR.z, stampRate), flash, 0.0f, RANK_BOX_HEIGHT / RANK_BOX_WIDTH);
		m_rankBox.Draw();
		m_rankLabel.SetPosition(Vector2(RANK_X, RANK_Y + RANK_LABEL_Y));
		m_rankLabel.Draw();
		m_rankLetter.SetPosition(Vector2(RANK_X, RANK_Y + RANK_LETTER_Y * scale), scale);
		m_rankLetter.Draw();
	}

	if (m_time >= INPUT_TIME)
	{
		const float blink = m_isLeaving ? 1.0f : 0.55f + 0.45f * sinf(m_time * BLINK_SPEED);
		m_press.SetColor(Vector4(PRESS_COLOR.x, PRESS_COLOR.y, PRESS_COLOR.z, blink));
		m_press.SetPosition(Vector2(0.0f, PRESS_Y));
		m_press.Draw();
	}
}
