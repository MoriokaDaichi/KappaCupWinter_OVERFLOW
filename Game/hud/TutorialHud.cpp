#include "stdafx.h"
#include "hud/TutorialHud.h"

#include "actor/BattleCharacter.h"
#include "param/GameParam.h"

namespace
{
	// Vector4 は constexpr にできないので、クラスの定数ではなくここに置く
	const Vector4 TEXT_COLOR(1.0f, 1.0f, 1.0f, 1.0f);		///< 案内の白
	const Vector4 ACCENT_COLOR(0.0f, 0.90f, 1.0f, 1.0f);	///< 帯の線と「GUIDE」のシアン（#00e5ff）
}

void TutorialHud::Init(const BattleCharacter* player)
{
	m_player = player;
	m_panel.Init("Assets/shader/hudTutorial.fx", PANEL_WIDTH, PANEL_HEIGHT);
	m_tab.Init("Assets/shader/hudTutorial.fx", TAB_WIDTH, TAB_HEIGHT);
	m_text.Init(TutorialParam::MAX_TEXT_LENGTH, TEXT_HEIGHT, HudText::Align::Left);
	m_label.Init(LABEL_LENGTH, LABEL_HEIGHT, HudText::Align::Left);
	Reset();
}

void TutorialHud::Update()
{
	if (m_player == nullptr)
	{
		return;
	}

	// 読み直し（F5）で数が変わっても外を読まないように、毎回ここから取る
	const TutorialParam& param = nsGameParam::Get().tutorial;
	const int count = static_cast<int>(param.messages.size());
	const float playerZ = m_player->GetPosition().z;

	// ワープで一度に何個も越えたときは、いちばん奥の案内だけを出す（手前のものはもう通り過ぎた場所なので出さない）
	while (m_nextIndex < count && playerZ >= param.messages[m_nextIndex].z)
	{
		m_shownIndex = m_nextIndex;
		m_elapsed = 0.0f;
		m_nextIndex++;
	}

	if (m_shownIndex < 0)
	{
		return;
	}

	m_elapsed += g_gameTime->GetFrameDeltaTime();
	if (m_elapsed >= param.showTime || m_shownIndex >= count)
	{
		m_shownIndex = -1;
	}
}

void TutorialHud::Draw()
{
	if (m_shownIndex < 0)
	{
		return;
	}

	const TutorialParam& param = nsGameParam::Get().tutorial;
	const std::string& text = param.messages[m_shownIndex].text;

	// 消える前の FADE_TIME 秒で、薄くしながら少し上へ逃がす
	const float remaining = param.showTime - m_elapsed;
	const float fade = min(remaining / FADE_TIME, 1.0f);
	const float panelY = PANEL_POSITION_Y + (1.0f - fade) * FADE_RISE;
	const float panelLeft = -PANEL_WIDTH * 0.5f;

	// 帯は左から伸び、伸びきってから文字を 1 文字ずつ打ち出す
	const float reveal = min(m_elapsed / PANEL_IN_TIME, 1.0f);
	Vector4 accent = ACCENT_COLOR;
	accent.w = fade;
	m_panel.Update(Vector2(0.0f, panelY), accent, reveal);
	m_panel.Draw();

	if (reveal < 1.0f)
	{
		return;
	}

	const int typed = static_cast<int>((m_elapsed - PANEL_IN_TIME) / CHAR_INTERVAL) + 1;
	const std::string shown = text.substr(0, min(static_cast<size_t>(typed), text.size()));
	Vector4 textColor = TEXT_COLOR;
	textColor.w = fade;
	m_text.SetText(shown.c_str());
	m_text.SetColor(textColor);
	m_text.SetPosition(Vector2(panelLeft + TEXT_LEFT_PADDING, panelY));
	m_text.Draw();

	// 帯の左上のタブ（背景が明るくても小さな文字が読めるように、帯と同じ紺を敷く）
	const float tabLeft = panelLeft + TAB_LEFT;
	const float tabY = panelY + (PANEL_HEIGHT + TAB_HEIGHT) * 0.5f + TAB_GAP;
	m_tab.Update(Vector2(tabLeft + TAB_WIDTH * 0.5f, tabY), accent, 1.0f);
	m_tab.Draw();

	char label[32];
	sprintf_s(label, "GUIDE %d/%d", m_shownIndex + 1, static_cast<int>(param.messages.size()));
	m_label.SetText(label);
	m_label.SetColor(accent);
	m_label.SetPosition(Vector2(tabLeft + LABEL_LEFT_PADDING, tabY));
	m_label.Draw();
}

void TutorialHud::Reset()
{
	m_nextIndex = 0;
	m_shownIndex = -1;
	m_elapsed = 0.0f;
}
