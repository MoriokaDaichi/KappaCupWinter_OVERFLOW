#pragma once
#include "hud/HudShape.h"
#include "hud/HudText.h"
#include "hud/HitMarker.h"
#include "hud/SpeedMeter.h"
#include "hud/TutorialHud.h"

class BattleCharacter;
class Checkpoint;
class GiantFirewall;

/**
 * ゲーム中の HUD（企画書の下書き「ゲーム画面」の配置）。Game が持つ
 * 左上にタイム、右上にチェックポイント、真ん中に照準、右下にスピードメーター、ロックしたアンカーに角カッコ、上のほうに案内を出す
 */
class Hud
{
public:
	/** 見るものを受け取る（どれも借りているだけ） */
	void Init(const BattleCharacter* player, const Checkpoint* checkpoint, const GiantFirewall* giantFirewall);
	/** 案内を進め、タイムを進める。タイムは巨大ファイアウォールを突破したら止める（毎フレーム Game の Update から呼ぶ） */
	void Update();
	/** タイムを 0 に戻し、案内を最初からにする（最初からやり直すとき） */
	void Reset() { m_time = 0.0f; m_levelTimeSum = 0.0f; m_tutorial.Reset(); m_hitMarker.Reset(); }
	/** 経過時間（秒）を返す。突破したら止まるので、クリアのタイムになる（リザルトに渡す） */
	float GetTime() const { return m_time; }
	/** タイムが進んでいた間の、速度 Lv の平均を返す（リザルトのランクに使う） */
	float GetAverageLevel() const { return (m_time > 0.0f) ? m_levelTimeSum / m_time : 1.0f; }
	/** 全部を描く（毎フレーム Game の Render から呼ぶ。全員が動いた後の位置に出すため） */
	void Draw();

private:
	/** 左上のタイムと、右上のチェックポイントを描く */
	void DrawInfo();
	/** ロックしたアンカーを角カッコで囲み、「[RB] GRAPPLE」を出す */
	void DrawGrappleMarker();
	/** 落ちてチェックポイントへ戻るまでの間、画面を暗くして赤い「SIGNAL LOST」を出す（企画書の失敗の絵） */
	void DrawSignalLost();

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
	static constexpr float SIGNAL_LOST_HEIGHT = 100.0f;	///< 「SIGNAL LOST」の文字の高さ（ピクセル）
	static constexpr float SIGNAL_LOST_JITTER = 14.0f;	///< 出た瞬間に「SIGNAL LOST」を左右にずらす幅（ピクセル）
	static constexpr float SIGNAL_LOST_SHAKE_STEPS = 40.0f;	///< 「SIGNAL LOST」を震わせる回数（出ている間の合計）
	static constexpr float SIGNAL_LOST_GHOST = 6.0f;	///< 後ろに重ねる暗い赤の文字のずれ（ピクセル）
	static constexpr int CHECKPOINT_LENGTH = 5;			///< チェックポイントの文字数（10/10 まで）

	const BattleCharacter* m_player = nullptr;			///< 見るプレイヤー（借りているだけ）
	const Checkpoint* m_checkpoint = nullptr;			///< 今のチェックポイントを聞く相手（借りているだけ）
	const GiantFirewall* m_giantFirewall = nullptr;		///< 突破したかを聞く相手（借りているだけ）
	float m_time = 0.0f;								///< 経過時間（秒）。突破したら止まる
	float m_levelTimeSum = 0.0f;						///< 速度 Lv × 経過時間 の合計（タイムで割ると平均の Lv）

	SpeedMeter m_speedMeter;							///< 右下のスピードメーター
	TutorialHud m_tutorial;								///< 上のほうの案内
	HitMarker m_hitMarker;								///< 弾が当たったときに照準のまわりに出す×
	HudShape m_crosshair;								///< 真ん中の照準（Assets/shader/hudCrosshair.fx）
	HudShape m_bracket;									///< ロックしたアンカーを囲む角カッコ（Assets/shader/hudBracket.fx）
	HudText m_timeLabel;								///< 「TIME」
	HudText m_timeValue;								///< タイムの数字
	HudText m_checkpointLabel;							///< 「CHECKPOINT」
	HudText m_checkpointValue;							///< チェックポイントの「3/6」
	HudShape m_signalLostScreen;						///< 落ちたときの暗い画面と赤い乱れ（Assets/shader/hudSignalLost.fx）
	HudText m_signalLostText;							///< 「SIGNAL LOST」
	HudText m_signalLostGhost;							///< 「SIGNAL LOST」の後ろに重ねる暗い赤（ずらしてグリッチに見せる）
	HudText m_grappleText;								///< 「[RB] GRAPPLE」
};
