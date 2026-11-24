#pragma once
#include "hud/HudShape.h"
#include "hud/HudText.h"

class Fade;

/**
 * タイトル画面。電脳空間の背景にタイトルの文字を出し、A（Start）でゲームへ進む
 * main が最初に作り、リザルトとポーズからも戻ってくる。ロゴとタイトルの演出は、それぞれのタスクで作り直す
 */
class TitleScene : public IGameObject
{
public:
	bool Start() override;
	void Update() override;
	void Render(RenderContext& rc) override;

private:
	static constexpr float FADE_TIME = 0.4f;			///< 黒くする・戻す秒数
	static constexpr float LOGO_HEIGHT = 150.0f;		///< タイトルの文字の高さ（ピクセル）
	static constexpr float LOGO_Y = 120.0f;				///< タイトルの文字の、画面の中央からの高さ
	static constexpr float SUB_HEIGHT = 26.0f;			///< タイトルの下の小さい文字の高さ
	static constexpr float SUB_Y = 20.0f;				///< タイトルの下の小さい文字の高さの位置
	static constexpr float PRESS_HEIGHT = 40.0f;		///< 「PRESS [A] START」の文字の高さ
	static constexpr float PRESS_Y = -170.0f;			///< 「PRESS [A] START」の位置
	static constexpr float KEY_HEIGHT = 22.0f;			///< キーボードの案内の文字の高さ
	static constexpr float KEY_Y = -215.0f;				///< キーボードの案内の位置
	static constexpr float BLINK_SPEED = 3.0f;			///< 「PRESS [A] START」の点滅の速さ（ラジアン / 秒）
	static constexpr float GRID_SPEED = 1.5f;			///< 背景の格子の流れる速さ

	Fade* m_fade = nullptr;				///< 画面を黒くする係（main が作ったものを借りる）
	bool m_isLeaving = false;			///< ボタンが押され、黒くしている途中か
	float m_time = 0.0f;				///< 出てからの秒数（点滅に使う）
	HudShape m_back;					///< 背景（Assets/shader/sceneBack.fx）
	HudText m_logo;						///< 「OVERFLOW」
	HudText m_sub;						///< 「FULL SPEED HACK」
	HudText m_press;					///< 「PRESS [A] START」
	HudText m_key;						///< 「KEY: ENTER / J」
};
