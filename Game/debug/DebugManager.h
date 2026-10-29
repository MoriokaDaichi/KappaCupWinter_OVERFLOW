#pragma once
#include "param/GameParam.h"

class BattleCharacter;

/**
 * デバッグ画面（エンジンの ImGui に「Game > OVERFLOW」のパネルを出す）。Game が持つ
 * 速度の表示と書き換え、Lv の強制変更、ワープ、JSON の読み直しを行う。F1 で画面ごと隠せる（PV の撮影用）
 */
class DebugManager
{
public:
	/** パネルを登録する。プレイヤーを作った後に、Game の Start から呼ぶ */
	void Init(BattleCharacter* player);
	/** 毎フレーム Game の Update から呼ぶ（F5 の読み直しと、速度の固定） */
	void Update();

private:
	/** パネルの中身を描く。ボタンやスライダーを動かしたときの処理もここで行う */
	void DrawPanel();
	/** 速度の表示・書き換え・Lv の強制変更 */
	void DrawSpeed();
	/** 状態・位置・速さの表示 */
	void DrawPlayer();
	/** ワープのボタン */
	void DrawWarp();
	/** JSON の読み直しのボタン */
	void DrawParam();
	/** 速度を決めて、固定しているなら固定する値も変える */
	void SetSpeed(const float base, const float bonus);

private:
	const DebugParam& m_param = nsGameParam::Get().debug;	///< ワープ先など（debug.json）
	const SpeedParam& m_speedParam = nsGameParam::Get().speed;	///< 速度の範囲と Lv の境目（speed.json）

	BattleCharacter* m_player = nullptr;			///< 調べる・書き換える相手（借りているだけ。消すのは Game）
	DebugGuiPanel m_panel;				///< パネルの登録。このクラスが消えると自動で外れる
	bool m_isReloadKeyDown = false;		///< 前のフレームで F5 を押していたか（押した瞬間だけ読み直すため）
	bool m_isSpeedLocked = false;		///< 速度を固定しているか（地上で上乗せが減らないようにする）
	float m_lockedBase = 0.0f;			///< 固定している基礎速度
	float m_lockedBonus = 0.0f;			///< 固定している上乗せ
};
