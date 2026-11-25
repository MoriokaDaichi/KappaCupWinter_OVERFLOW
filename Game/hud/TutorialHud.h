#pragma once
#include "hud/HudShape.h"
#include "hud/HudText.h"
#include "hud/JapaneseText.h"

class BattleCharacter;

/**
 * 各要素が初めて出る場所で、短い案内を画面の上のほうに数秒出す（tutorial.json）。Hud が持つ
 * 斜めに切った紺の帯が左から伸び、日本語の文字が 1 文字ずつ打ち出される。帯の左上に小さな帯（タブ）を付け、「GUIDE 3/8」を出す
 * 案内は z の小さい順に並んでいる（起動時に GameParam が確かめる）ので、「次の案内」を 1 つ見ていれば足りる
 */
class TutorialHud
{
public:
	/** 見るプレイヤーを受け取る（借りているだけ） */
	void Init(const BattleCharacter* player);
	/** プレイヤーが次の案内の場所を越えたら出し、出している時間を進める（毎フレーム Hud の Update から呼ぶ） */
	void Update();
	/** 出している間だけ描く（毎フレーム Hud の Draw から呼ぶ） */
	void Draw(RenderContext& rc);
	/** 最初の案内からやり直す（最初からやり直すとき）。チェックポイントへ戻すときは呼ばない（同じ案内を二度出さないため） */
	void Reset();

private:
	static constexpr float PANEL_WIDTH = 620.0f;		///< 帯の幅（ピクセル。日本語 16 文字が入る幅）
	static constexpr float PANEL_HEIGHT = 72.0f;		///< 帯の高さ（ピクセル）
	static constexpr float PANEL_POSITION_Y = 250.0f;	///< 帯の真ん中の高さ（画面の中央から上へ。照準とタイムの間）
	static constexpr float TEXT_HEIGHT = 38.0f;			///< 案内の文字の高さ（ピクセル）
	static constexpr float TEXT_LEFT_PADDING = 44.0f;	///< 帯の左端から文字までの空き（ピクセル。左の印と斜めの切り口の分）
	static constexpr float LABEL_HEIGHT = 20.0f;		///< 「GUIDE 3/8」の文字の高さ（ピクセル）
	static constexpr float TAB_WIDTH = 190.0f;			///< 「GUIDE 3/8」を載せるタブの幅（ピクセル）
	static constexpr float TAB_HEIGHT = 32.0f;			///< タブの高さ（ピクセル）
	static constexpr float TAB_LEFT = 14.0f;			///< タブの左端の、帯の左端からのずれ（ピクセル。斜めの切り口にそろえる）
	static constexpr float TAB_GAP = 4.0f;				///< タブと帯の間の空き（ピクセル）
	static constexpr float LABEL_LEFT_PADDING = 26.0f;	///< タブの左端から「GUIDE」までの空き（ピクセル）
	static constexpr int LABEL_LENGTH = 11;				///< 「GUIDE 10/10」までの文字数
	static constexpr float PANEL_IN_TIME = 0.18f;		///< 帯が左から伸びきるまでの秒数
	static constexpr float CHAR_INTERVAL = 0.03f;		///< 文字を 1 文字打ち出す間隔（秒）
	static constexpr float FADE_TIME = 0.4f;			///< 消える前に薄くしていく秒数
	static constexpr float FADE_RISE = 12.0f;			///< 消えるときに上へずらす距離（ピクセル）

	const BattleCharacter* m_player = nullptr;			///< 見るプレイヤー（借りているだけ）
	int m_nextIndex = 0;								///< まだ出していない案内のうち、いちばん手前のものの番号
	int m_shownIndex = -1;								///< 今出している案内の番号（-1 なら出していない）
	float m_elapsed = 0.0f;								///< 今の案内を出してからの秒数
	HudShape m_panel;									///< 帯（Assets/shader/hudTutorial.fx）
	HudShape m_tab;										///< 「GUIDE 3/8」を載せるタブ（帯と同じシェーダー）
	JapaneseText m_text;								///< 案内の文字（日本語）
	HudText m_label;									///< 「GUIDE 3/8」
};
