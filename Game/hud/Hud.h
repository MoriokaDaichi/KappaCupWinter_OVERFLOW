#pragma once
#include "hud/HudShape.h"
#include "hud/HudText.h"
#include "hud/SpeedMeter.h"

class BattleCharacter;
class Checkpoint;
class GiantFirewall;

/**
 * ゲーム中の HUD（企画書の下書き「ゲーム画面」の配置）。Game が持つ
 * 左上にタイム、右上にチェックポイント、真ん中に照準、右下にスピードメーター、ロックしたアンカーに角カッコを出す
 */
class Hud
{
public:
	/** 見るものを受け取る（どれも借りているだけ） */
	void Init(const BattleCharacter* player, const Checkpoint* checkpoint, const GiantFirewall* giantFirewall);
	/** タイムを進める。巨大ファイアウォールを突破したら止める（毎フレーム Game の Update から呼ぶ） */
	void Update();
	/** タイムを 0 に戻す（最初からやり直すとき） */
	void Reset() { m_time = 0.0f; }
	/** 全部を描く（毎フレーム Game の Render から呼ぶ。全員が動いた後の位置に出すため） */
	void Draw();

private:
	/** 左上のタイムと、右上のチェックポイントを描く */
	void DrawInfo();
	/** ロックしたアンカーを角カッコで囲み、「[RB] GRAPPLE」を出す */
	void DrawGrappleMarker();

private:
	static constexpr float SCREEN_MARGIN = 40.0f;		///< 画面の端からの空き（ピクセル）
	static constexpr float LABEL_HEIGHT = 22.0f;		///< 「TIME」「CHECKPOINT」の文字の高さ（ピクセル）
	static constexpr float VALUE_HEIGHT = 48.0f;		///< タイムとチェックポイントの数字の高さ（ピクセル）
	static constexpr float VALUE_OFFSET_Y = 44.0f;		///< 数字の真ん中の、見出しの真ん中からの下がり（ピクセル）
	static constexpr float CROSSHAIR_SIZE = 48.0f;		///< 照準の大きさ（ピクセル）
	static constexpr float BRACKET_SIZE = 90.0f;		///< アンカーを囲む角カッコの大きさ（ピクセル）
	static constexpr float GRAPPLE_TEXT_OFFSET_X = 60.0f;	///< 「[RB] GRAPPLE」の、角カッコの中心から右へのずれ（ピクセル）
	static constexpr float GRAPPLE_TEXT_HEIGHT = 22.0f;	///< 「[RB] GRAPPLE」の文字の高さ（ピクセル）
	static constexpr float SECONDS_PER_MINUTE = 60.0f;	///< 1 分の秒数
	static constexpr int TIME_LENGTH = 8;				///< タイムの文字数（00:00.00）
	static constexpr int CHECKPOINT_LENGTH = 5;			///< チェックポイントの文字数（10/10 まで）

	const BattleCharacter* m_player = nullptr;			///< 見るプレイヤー（借りているだけ）
	const Checkpoint* m_checkpoint = nullptr;			///< 今のチェックポイントを聞く相手（借りているだけ）
	const GiantFirewall* m_giantFirewall = nullptr;		///< 突破したかを聞く相手（借りているだけ）
	float m_time = 0.0f;								///< 経過時間（秒）。突破したら止まる

	SpeedMeter m_speedMeter;							///< 右下のスピードメーター
	HudShape m_crosshair;								///< 真ん中の照準（Assets/shader/hudCrosshair.fx）
	HudShape m_bracket;									///< ロックしたアンカーを囲む角カッコ（Assets/shader/hudBracket.fx）
	HudText m_timeLabel;								///< 「TIME」
	HudText m_timeValue;								///< タイムの数字
	HudText m_checkpointLabel;							///< 「CHECKPOINT」
	HudText m_checkpointValue;							///< チェックポイントの「3/6」
	HudText m_grappleText;								///< 「[RB] GRAPPLE」
};
