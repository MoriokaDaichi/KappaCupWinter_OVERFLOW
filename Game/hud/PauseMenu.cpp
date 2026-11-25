#include "stdafx.h"
#include "hud/PauseMenu.h"

#include "input/InputManager.h"

namespace
{
	// Vector4 は constexpr にできないので、クラスの定数ではなくここに置く
	const Vector4 DIM_COLOR(0.0f, 0.0f, 0.05f, 0.65f);			///< ゲームの画面を暗くする色
	const Vector4 HEADER_COLOR(0.0f, 0.9f, 1.0f, 1.0f);			///< 「PAUSE」のシアン
	const Vector4 SELECTED_COLOR(1.0f, 1.0f, 1.0f, 1.0f);		///< 選んでいるものの白
	const Vector4 UNSELECTED_COLOR(0.5f, 0.6f, 0.7f, 1.0f);		///< 選んでいないものの灰色
	const Vector4 CURSOR_COLOR(0.0f, 0.9f, 1.0f, 1.0f);			///< 帯の上下の線のシアン
	const Vector4 GUIDE_COLOR(0.6f, 0.7f, 0.8f, 1.0f);			///< 操作の案内の灰色

	// 並びは PauseMenu::Choice の Resume からの順と同じ。「やり直す」はチェックポイントからやり直す
	const char* ITEM_TEXTS[] = { u8"再開", u8"やり直す", u8"タイトルに戻る" };
}

void PauseMenu::Init()
{
	static_assert(ARRAYSIZE(ITEM_TEXTS) == ITEM_NUM, "ITEM_TEXTS の数を ITEM_NUM と合わせてください");

	m_dim.Init("Assets/shader/hudFill.fx", UI_SPACE_WIDTH, UI_SPACE_HEIGHT);
	m_cursor.Init("Assets/shader/hudTutorial.fx", CURSOR_WIDTH, CURSOR_HEIGHT);

	const char* header = "PAUSE";
	m_header.Init(static_cast<int>(strlen(header)), HEADER_HEIGHT, HudText::Align::Center);
	m_header.SetText(header);
	m_header.SetColor(HEADER_COLOR);
	m_header.SetPosition(Vector2(0.0f, HEADER_Y));

	for (int i = 0; i < ITEM_NUM; i++)
	{
		m_items[i].Init(ITEM_HEIGHT, JapaneseText::Align::Center);
		m_items[i].SetText(ITEM_TEXTS[i]);
		m_items[i].SetPosition(Vector2(0.0f, ITEM_TOP_Y - ITEM_SPACING * i));
	}

	m_guide.Init(GUIDE_HEIGHT, JapaneseText::Align::Center);
	m_guide.SetText(u8"[A] 決定　[B] 戻る");
	m_guide.SetColor(GUIDE_COLOR);
	m_guide.SetPosition(Vector2(0.0f, GUIDE_Y));
}

void PauseMenu::Open()
{
	m_selector.Reset(0);
}

PauseMenu::Choice PauseMenu::Update(const InputManager& input)
{
	m_selector.Update(input, ITEM_NUM);

	// B と、開いたときと同じ Start は「再開」（閉じる）
	if (input.IsTrigger(EnInputAction::Cancel) || input.IsTrigger(EnInputAction::Pause))
	{
		return Choice::Resume;
	}

	if (input.IsTrigger(EnInputAction::Jump))
	{
		// 0 番が Resume なので、1 つずらして Choice にする
		return static_cast<Choice>(m_selector.GetIndex() + static_cast<int>(Choice::Resume));
	}

	return Choice::None;
}

void PauseMenu::Draw(RenderContext& rc)
{
	m_dim.Update(Vector2::Zero, DIM_COLOR, 0.0f);
	m_dim.Draw();

	m_header.Draw();

	// 帯を先に描き、その上に文字を重ねる
	const int selected = m_selector.GetIndex();
	m_cursor.Update(Vector2(0.0f, ITEM_TOP_Y - ITEM_SPACING * selected), CURSOR_COLOR, 1.0f);
	m_cursor.Draw();

	for (int i = 0; i < ITEM_NUM; i++)
	{
		m_items[i].SetColor(i == selected ? SELECTED_COLOR : UNSELECTED_COLOR);
		m_items[i].Draw(rc);
	}

	m_guide.Draw(rc);
}
