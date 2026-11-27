#pragma once
#include "hud/HudShape.h"
#include "hud/HudText.h"
#include "system/TkmPrefetcher.h"
#include <array>

class Fade;
class Game;

/**
 * タイトル画面（企画書の「スタート」の絵）
 * 主人公の手（3D。title_arms.tkm）が持った携帯端末（3D。title_tablet.tkm）の画面にコードが流れ、グリッチしたロゴと「> PRESS START_」が出る
 * カメラは主人公の目の位置（原点）。画面の中身は 2D なので、3D の端末の画面の四隅を画面の上の位置に直して、そこへ重ねる
 * A（Start）を押すと親指で画面を押し、画面が光って吸い込まれ、マゼンタの通路を落ちて（ダイブ）、ゲームが始まる
 * main が最初に作り、リザルトとポーズからも戻ってくる
 */
class TitleScene : public IGameObject
{
public:
	bool Start() override;
	void Update() override;
	void Render(RenderContext& rc) override;

private:
	/** 今どこまで進んだか */
	enum class Phase : int
	{
		Idle,		///< 押されるのを待っている
		Tap,		///< 親指で押している
		Flash,		///< 画面が白く光る
		Dive,		///< 通路を落ちていく
		Leave,		///< 黒くしてゲームへ移る
	};

	/** カメラとライトを決め、腕・端末・背景の 3D モデルを作る */
	void InitModels();
	/** 3D の端末の画面が、画面の上のどこに映るかを出す（2D の中身をそこへ重ねるため）。カメラを決めた後に呼ぶ */
	void CalcPanelRect();
	/** phase に進み、その経過時間を 0 にする */
	void ChangePhase(const Phase phase);
	/** 画面の中のコードの文字を、時間に合わせて作り直す（打ち込むように 1 文字ずつ出す・数字を変える） */
	void UpdateCode();
	/** 端末の画面の中（コード・ロゴ・PRESS START）を描く。alpha で全体を薄くする */
	void DrawPanel(RenderContext& rc, const float alpha);
	/** 端末のまわりに浮かぶ数字を描く */
	void DrawFloatingDigits();
	/** ダイブ中に、真ん中から外へ飛んでいく数字を描く */
	void DrawDiveDigits();

private:
	static constexpr int CODE_LINE_NUM = 8;				///< 画面の中のコードの行の数（上に 4 行、下に 4 行）
	static constexpr int FLOAT_DIGIT_NUM = 6;			///< 端末のまわりに浮かぶ数字の数
	static constexpr int DIVE_DIGIT_NUM = 14;			///< ダイブ中に飛んでいく数字の数
	static constexpr float FADE_IN_TIME = 0.4f;			///< 出るときに黒から戻す秒数
	static constexpr float TAP_TIME = 0.25f;			///< 親指で押す秒数
	static constexpr float FLASH_TIME = 0.35f;			///< 画面が白く光る秒数
	static constexpr float DIVE_TIME = 1.8f;			///< 通路を落ちる秒数
	static constexpr float LEAVE_TIME = 0.35f;			///< 黒くする秒数
	static constexpr float CODE_HEIGHT = 24.0f;			///< コードの文字の高さ（ピクセル）
	static constexpr float CODE_LEFT = -470.0f;			///< コードの左端（端末の画面の中心からのずれ。以下の位置も同じ）
	static constexpr float CODE_TOP_Y = 262.0f;			///< 上のコードのいちばん上の行の高さ
	static constexpr float CODE_BOTTOM_Y = -138.0f;		///< 下のコードのいちばん上の行の高さ
	static constexpr float CODE_SPACING = 34.0f;		///< コードの行の間隔
	static constexpr float CODE_LINE_DELAY = 0.12f;		///< 行を 1 つずつ打ち始める間隔（秒）
	static constexpr float CODE_TYPE_SPEED = 90.0f;		///< 1 秒に打つ文字の数
	static constexpr float CODE_REFRESH_TIME = 0.12f;	///< 変わる数字（16 進の行）を作り直す間隔（秒）
	static constexpr float LOGO_WIDTH = 1000.0f;		///< ロゴの画像の大きさ（画像は 2064 x 512）
	static constexpr float LOGO_HEIGHT = 248.0f;
	static constexpr float LOGO_Y = 55.0f;				///< ロゴの高さ
	static constexpr float LOGO_GLITCH_SCALE = 0.89f;	///< グリッチ版の表示の大きさ（グリッチ版は文字が大きく描かれているので、本番の版と文字の大きさをそろえる）
	static constexpr int LOGO_SHAKE = 6;				///< 乱れている間に、ロゴを左右に震わせる幅（ピクセル）
	static constexpr float GLITCH_INTERVAL = 1.7f;		///< ロゴが乱れる間隔（秒）
	static constexpr float GLITCH_TIME = 0.15f;			///< ロゴが乱れている秒数
	static constexpr float GLITCH_WIDTH = 1000.0f;		///< ロゴに重ねる帯の板の大きさ
	static constexpr float GLITCH_HEIGHT = 220.0f;
	static constexpr float PRESS_HEIGHT = 44.0f;		///< 「> PRESS START_」の文字の高さ
	static constexpr float PRESS_Y = -65.0f;			///< 「> PRESS START_」の高さ
	static constexpr float CURSOR_BLINK_TIME = 0.5f;	///< 「_」が点滅する間隔（秒）
	static constexpr float KEY_HEIGHT = 22.0f;			///< キーボードの案内の文字の高さ
	static constexpr float KEY_Y = -500.0f;				///< キーボードの案内の位置
	static constexpr float FLOAT_DIGIT_HEIGHT = 30.0f;	///< 浮かぶ数字の高さ
	static constexpr float DIVE_DIGIT_HEIGHT = 40.0f;	///< ダイブ中の数字の高さ
	static constexpr float CAMERA_VIEW_ANGLE = 34.5f;	///< カメラの縦の視野角（度）。端末の画面が、画面の高さの 6 割くらいに映る
	static constexpr float CAMERA_NEAR = 1.0f;			///< カメラの手前の切り口（端末は目の前 40 なので、ゲームより近くまで描く）
	static constexpr float CAMERA_FAR = 2000.0f;		///< カメラの奥の切り口（背景の板は 500 先）
	static constexpr float EXPOSURE_SCALE = 0.3f;		///< 自動露出の目標の明るさに掛ける値（ステージと同じ。暗い場面を白っぽく持ち上げないように）
	static constexpr float LIGHT_BRIGHTNESS = 3.0f;		///< 腕に当てるライトの明るさ
	static constexpr float TOUCH_SIZE = 220.0f;			///< 親指のまわりの輪の板の大きさ（ピクセル）

	Fade* m_fade = nullptr;				///< 画面を黒くする係（main が作ったものを借りる）
	Phase m_phase = Phase::Idle;		///< 今どこまで進んだか
	float m_time = 0.0f;				///< 出てからの秒数
	float m_phaseTime = 0.0f;			///< 今の Phase になってからの秒数
	float m_codeRefreshTimer = 0.0f;	///< 変わる数字を作り直すまでの残り（秒）
	Game* m_game = nullptr;				///< ダイブ中に作り始めたゲーム（準備ができたか聞き、Begin を呼ぶ。消すのはゲーム自身）
	TkmPrefetcher m_prefetcher;			///< ゲームで使う重い tkm を、タイトルの間に別スレッドで読んでおく係

	/** titleBack.fx に渡す値（titleBack.fx の BackCb と同じ並び） */
	struct BackConstant
	{
		float time = 0.0f;			///< 経過時間（秒）
		float pad[3] = {};			///< 16 バイトにそろえるための詰め物
	};

	ModelRender m_arms;					///< 端末を持つ主人公の腕と手（title_arms.tkm。Blender でポーズを付けた静止モデル。titleArms.fx）
	ModelRender m_tablet;				///< 端末（title_tablet.tkm。紺にシアンの縁。titleTablet.fx）
	ModelRender m_back;					///< 背景の板（title_back.tkm。titleBack.fx）
	BackConstant m_backConstant;		///< 背景に渡す値
	Vector2 m_panelCenter;				///< 端末の画面の中心が映る位置（ピクセル。画面の中央が 0,0）
	Vector2 m_panelSize;				///< 端末の画面が映る大きさ（ピクセル）
	Vector2 m_touchPosition;			///< 親指の先が映る位置（輪を出す）
	HudShape m_panel;					///< 端末の画面（Assets/shader/titlePanel.fx）
	HudShape m_touch;					///< 親指のまわりの輪（Assets/shader/titleTouch.fx）
	HudShape m_glitch;					///< ロゴに重ねる帯（Assets/shader/titleGlitch.fx）
	HudShape m_dive;					///< ダイブの通路（Assets/shader/titleDive.fx）
	std::array<HudText, CODE_LINE_NUM> m_code;			///< 画面の中のコードの行
	std::array<std::string, CODE_LINE_NUM> m_codeText;	///< コードの行の、全部打ち終わったときの文字
	SpriteRender m_logo;				///< ロゴの本番の版（logo_final.DDS。画像生成 AI で作り、D:\my\05_Assets\02_OVERFLOW\logo\ai の black_to_alpha.py で黒を透明にした）
	SpriteRender m_logoGlitch;			///< ロゴのグリッチ版（logo_glitch.DDS。作り方は同じ）
	HudText m_press;					///< 「> PRESS START_」
	HudText m_key;						///< キーボードの案内
	std::array<HudText, FLOAT_DIGIT_NUM> m_floatDigits;	///< 端末のまわりに浮かぶ数字
	std::array<HudText, DIVE_DIGIT_NUM> m_diveDigits;	///< ダイブ中に飛んでいく数字
};
