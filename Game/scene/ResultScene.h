#pragma once
#include "hud/HudShape.h"
#include "hud/HudText.h"

class Fade;

/**
 * リザルト画面。クリアしたタイムと倒した数を出し、A（Start）でタイトルへ戻る
 * Game がゴールの後に作り、SetResult で結果を渡す。ランクとリザルトの演出は、それぞれのタスクで足す
 */
class ResultScene : public IGameObject
{
public:
	bool Start() override;
	void Update() override;
	void Render(RenderContext& rc) override;

public:
	/** 結果を受け取る（NewGO の直後、Start より前に呼ぶ） */
	void SetResult(const float clearTime, const int defeatCount);

private:
	/** 見出しと値を 1 行出す（左に見出し、右に値） */
	void DrawRow(HudText& label, HudText& value, const float y);

private:
	static constexpr float FADE_TIME = 0.4f;			///< 黒くする・戻す秒数
	static constexpr float INPUT_WAIT_TIME = 0.8f;		///< 出てからこの秒数はボタンを受け付けない（ゴールで押していたボタンで、すぐ抜けないように）
	static constexpr float HEADER_HEIGHT = 90.0f;		///< 「RESULT」の文字の高さ
	static constexpr float HEADER_Y = 230.0f;			///< 「RESULT」の位置
	static constexpr float LABEL_HEIGHT = 34.0f;		///< 見出しの文字の高さ
	static constexpr float VALUE_HEIGHT = 64.0f;		///< 値の文字の高さ
	static constexpr float ROW_LEFT = -360.0f;			///< 見出しの左端
	static constexpr float ROW_RIGHT = 360.0f;			///< 値の右端
	static constexpr float TIME_Y = 70.0f;				///< タイムの行の高さ
	static constexpr float DEFEAT_Y = -40.0f;			///< 倒した数の行の高さ
	static constexpr float PRESS_HEIGHT = 36.0f;		///< 「PRESS [A] TITLE」の文字の高さ
	static constexpr float PRESS_Y = -220.0f;			///< 「PRESS [A] TITLE」の位置
	static constexpr float BLINK_SPEED = 3.0f;			///< 点滅の速さ（ラジアン / 秒）
	static constexpr float GRID_SPEED = 0.6f;			///< 背景の格子の流れる速さ（タイトルよりゆっくり）
	static constexpr float SECONDS_PER_MINUTE = 60.0f;	///< 1 分の秒数

	Fade* m_fade = nullptr;				///< 画面を黒くする係（main が作ったものを借りる）
	bool m_isLeaving = false;			///< ボタンが押され、黒くしている途中か
	float m_time = 0.0f;				///< 出てからの秒数
	float m_clearTime = 0.0f;			///< クリアしたタイム（秒）
	int m_defeatCount = 0;				///< 倒した敵の数
	HudShape m_back;					///< 背景（Assets/shader/sceneBack.fx）
	HudText m_header;					///< 「RESULT」
	HudText m_timeLabel;				///< 「TIME」
	HudText m_timeValue;				///< タイムの数字
	HudText m_defeatLabel;				///< 「DEFEATS」
	HudText m_defeatValue;				///< 倒した数
	HudText m_press;					///< 「PRESS [A] TITLE」
};
