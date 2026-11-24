#pragma once
#include "hud/HudShape.h"
#include "hud/HudText.h"
#include "scene/MenuSelector.h"

class InputManager;

/**
 * ポーズのメニュー（再開・チェックポイントから・タイトルへ）。ゲームの画面を暗くして、真ん中に並べる
 * 選ぶ・決めるだけを受け持ち、決めた後に何をするかは Game が決める。Game が持つ
 */
class PauseMenu
{
public:
	/** 選んだもの */
	enum class Choice : int
	{
		None,			///< まだ決めていない
		Resume,			///< 再開
		Checkpoint,		///< チェックポイントからやり直す
		Title,			///< タイトルへ戻る
	};

	/** 文字と板を作る */
	void Init();
	/** 開いたときに呼ぶ（いちばん上の「再開」を選んだ状態にする） */
	void Open();
	/** 入力を見て選ぶ。決めたら、その Choice を返す（B・Start は「再開」） */
	Choice Update(const InputManager& input);
	/** 描く予約をする（毎フレーム、開いている間だけ Game の Render から呼ぶ） */
	void Draw();

private:
	static constexpr int ITEM_NUM = 3;					///< 並べるものの数
	static constexpr float HEADER_HEIGHT = 72.0f;		///< 「PAUSE」の文字の高さ
	static constexpr float HEADER_Y = 170.0f;			///< 「PAUSE」の位置
	static constexpr float ITEM_HEIGHT = 36.0f;			///< 並べるものの文字の高さ
	static constexpr float ITEM_TOP_Y = 40.0f;			///< いちばん上のものの位置
	static constexpr float ITEM_SPACING = 80.0f;		///< 並べる間隔
	static constexpr float CURSOR_WIDTH = 460.0f;		///< 選んでいるものの下に敷く帯の幅
	static constexpr float CURSOR_HEIGHT = 60.0f;		///< 同じく高さ
	static constexpr float GUIDE_HEIGHT = 22.0f;		///< 操作の案内の文字の高さ
	static constexpr float GUIDE_Y = -230.0f;			///< 操作の案内の位置

	MenuSelector m_selector;							///< 今どれを選んでいるか
	HudShape m_dim;										///< ゲームの画面を暗くする板（Assets/shader/hudFill.fx）
	HudShape m_cursor;									///< 選んでいるものの下の帯（チュートリアルの帯と同じ見た目）
	HudText m_header;									///< 「PAUSE」
	HudText m_items[ITEM_NUM];							///< 「RESUME」「CHECKPOINT」「TITLE」
	HudText m_guide;									///< 「[A] OK  [B] BACK」
};
