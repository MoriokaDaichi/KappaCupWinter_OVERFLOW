#include "stdafx.h"
#include "scene/ResultScene.h"

#include "input/InputManager.h"
#include "scene/Fade.h"
#include "scene/TitleScene.h"

namespace
{
	// Vector4 は constexpr にできないので、クラスの定数ではなくここに置く
	const Vector4 GRID_COLOR(1.0f, 0.17f, 0.84f, 1.0f);		///< 背景の格子のマゼンタ（タイトルと見分ける）
	const Vector4 HEADER_COLOR(1.0f, 0.17f, 0.84f, 1.0f);	///< 「RESULT」のマゼンタ
	const Vector4 LABEL_COLOR(0.0f, 0.9f, 1.0f, 1.0f);		///< 見出しのシアン（HUD と同じ）
	const Vector4 VALUE_COLOR(1.0f, 1.0f, 1.0f, 1.0f);		///< 値の白
	const Vector4 PRESS_COLOR(1.0f, 1.0f, 1.0f, 1.0f);		///< 「PRESS [A] TITLE」の白
}

void ResultScene::SetResult(const float clearTime, const int defeatCount)
{
	m_clearTime = clearTime;
	m_defeatCount = defeatCount;
}

bool ResultScene::Start()
{
	m_fade = FindGO<Fade>(Fade::NAME);
	if (m_fade != nullptr)
	{
		m_fade->StartFadeIn(FADE_TIME);
	}

	m_back.Init("Assets/shader/sceneBack.fx", UI_SPACE_WIDTH, UI_SPACE_HEIGHT);

	// HudText は 1 つに 16 文字まで
	const char* header = "RESULT";
	const char* timeLabel = "TIME";
	const char* defeatLabel = "DEFEATS";
	const char* press = "PRESS [A] TITLE";

	m_header.Init(static_cast<int>(strlen(header)), HEADER_HEIGHT, HudText::Align::Center);
	m_header.SetText(header);
	m_header.SetColor(HEADER_COLOR);
	m_header.SetPosition(Vector2(0.0f, HEADER_Y));

	m_timeLabel.Init(static_cast<int>(strlen(timeLabel)), LABEL_HEIGHT, HudText::Align::Left);
	m_timeLabel.SetText(timeLabel);
	m_timeLabel.SetColor(LABEL_COLOR);

	const int minutes = static_cast<int>(m_clearTime / SECONDS_PER_MINUTE);
	const float seconds = m_clearTime - minutes * SECONDS_PER_MINUTE;
	char timeText[32];
	sprintf_s(timeText, "%02d:%05.2f", minutes, seconds);
	m_timeValue.Init(static_cast<int>(strlen(timeText)), VALUE_HEIGHT, HudText::Align::Right);
	m_timeValue.SetText(timeText);
	m_timeValue.SetColor(VALUE_COLOR);

	m_defeatLabel.Init(static_cast<int>(strlen(defeatLabel)), LABEL_HEIGHT, HudText::Align::Left);
	m_defeatLabel.SetText(defeatLabel);
	m_defeatLabel.SetColor(LABEL_COLOR);

	char defeatText[16];
	sprintf_s(defeatText, "%d", m_defeatCount);
	m_defeatValue.Init(static_cast<int>(strlen(defeatText)), VALUE_HEIGHT, HudText::Align::Right);
	m_defeatValue.SetText(defeatText);
	m_defeatValue.SetColor(VALUE_COLOR);

	m_press.Init(static_cast<int>(strlen(press)), PRESS_HEIGHT, HudText::Align::Center);
	m_press.SetText(press);
	m_press.SetPosition(Vector2(0.0f, PRESS_Y));

	return true;
}

void ResultScene::Update()
{
	InputManager& input = InputManager::GetInstance();
	input.Update();
	m_time += g_gameTime->GetFrameDeltaTime();

	const bool canInput = m_time >= INPUT_WAIT_TIME;
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

void ResultScene::DrawRow(HudText& label, HudText& value, const float y)
{
	label.SetPosition(Vector2(ROW_LEFT, y));
	value.SetPosition(Vector2(ROW_RIGHT, y));
	label.Draw();
	value.Draw();
}

void ResultScene::Render(RenderContext& rc)
{
	m_back.Update(Vector2::Zero, GRID_COLOR, GRID_SPEED);
	m_back.Draw();

	m_header.Draw();
	DrawRow(m_timeLabel, m_timeValue, TIME_Y);
	DrawRow(m_defeatLabel, m_defeatValue, DEFEAT_Y);

	// 受け付けるまでは出さない（押せるようになったことが分かるように）
	if (m_time >= INPUT_WAIT_TIME)
	{
		const float blink = m_isLeaving ? 1.0f : 0.55f + 0.45f * sinf(m_time * BLINK_SPEED);
		m_press.SetColor(Vector4(PRESS_COLOR.x, PRESS_COLOR.y, PRESS_COLOR.z, blink));
		m_press.Draw();
	}
}
