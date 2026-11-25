#pragma once
#include "hud/HudShape.h"
#include "hud/HudText.h"
#include <array>

class Fade;

/**
 * タイトル画面（企画書の「スタート」の絵）
 * 手に持った携帯端末の画面にコードが流れ、グリッチしたロゴと「> PRESS START_」が出る
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
	static constexpr float CODE_LEFT = -500.0f;			///< コードの左端（画面の中央からのずれ）
	static constexpr float CODE_TOP_Y = 285.0f;			///< 上のコードのいちばん上の行の高さ
	static constexpr float CODE_BOTTOM_Y = -115.0f;		///< 下のコードのいちばん上の行の高さ
	static constexpr float CODE_SPACING = 34.0f;		///< コードの行の間隔
	static constexpr float CODE_LINE_DELAY = 0.12f;		///< 行を 1 つずつ打ち始める間隔（秒）
	static constexpr float CODE_TYPE_SPEED = 90.0f;		///< 1 秒に打つ文字の数
	static constexpr float CODE_REFRESH_TIME = 0.12f;	///< 変わる数字（16 進の行）を作り直す間隔（秒）
	static constexpr float LOGO_WIDTH = 1000.0f;		///< ロゴの画像の大きさ（画像は 2064 x 512）
	static constexpr float LOGO_HEIGHT = 248.0f;
	static constexpr float LOGO_Y = 75.0f;				///< ロゴの高さ
	static constexpr float LOGO_GLITCH_SCALE = 0.89f;	///< グリッチ版の表示の大きさ（グリッチ版は文字が大きく描かれているので、本番の版と文字の大きさをそろえる）
	static constexpr int LOGO_SHAKE = 6;				///< 乱れている間に、ロゴを左右に震わせる幅（ピクセル）
	static constexpr float GLITCH_INTERVAL = 1.7f;		///< ロゴが乱れる間隔（秒）
	static constexpr float GLITCH_TIME = 0.15f;			///< ロゴが乱れている秒数
	static constexpr float GLITCH_WIDTH = 1000.0f;		///< ロゴに重ねる帯の板の大きさ
	static constexpr float GLITCH_HEIGHT = 220.0f;
	static constexpr float PRESS_HEIGHT = 44.0f;		///< 「> PRESS START_」の文字の高さ
	static constexpr float PRESS_Y = -45.0f;			///< 「> PRESS START_」の高さ
	static constexpr float CURSOR_BLINK_TIME = 0.5f;	///< 「_」が点滅する間隔（秒）
	static constexpr float KEY_HEIGHT = 22.0f;			///< キーボードの案内の文字の高さ
	static constexpr float KEY_Y = -500.0f;				///< キーボードの案内の位置
	static constexpr float FLOAT_DIGIT_HEIGHT = 30.0f;	///< 浮かぶ数字の高さ
	static constexpr float DIVE_DIGIT_HEIGHT = 40.0f;	///< ダイブ中の数字の高さ

	Fade* m_fade = nullptr;				///< 画面を黒くする係（main が作ったものを借りる）
	Phase m_phase = Phase::Idle;		///< 今どこまで進んだか
	float m_time = 0.0f;				///< 出てからの秒数
	float m_phaseTime = 0.0f;			///< 今の Phase になってからの秒数
	float m_codeRefreshTimer = 0.0f;	///< 変わる数字を作り直すまでの残り（秒）
	bool m_isLoadingShown = false;	///< 真っ黒の上に「> DIVING...」を出したか
	bool m_isGame = false;				///< もうゲームを作ったか（2 回作らないように）

	HudShape m_device;					///< 端末と手（Assets/shader/titleDevice.fx）
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
