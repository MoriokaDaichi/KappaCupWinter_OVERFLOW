#pragma once
#include "param/GameParam.h"

/** 速度の管理役。基礎速度と上乗せを持ち、合計から Lv を決める（Player が持つ） */
class SpeedSystem
{
public:
	/** 基礎速度をbaseMinに、上乗せ速度を0にする（Player の Start で呼ぶ） */
	void Reset();
	/** 基礎速度を増やす（マイナスで減らす）baseMin～baseMaxに収める */
	void AddBase(float amount);
	/** 上乗せを増やす（マイナスで減らす）0～bonusMaxに収める */
	void AddBonus(float amount);
	/** 上乗せが bonus より小さいときだけ、bonus にする（スイングを離したときに使う。上限で収める） */
	void RaiseBonusTo(float bonus);
	/** 基礎速度 ＋ 上乗せ */
	float GetSpeed() const { return m_baseSpeed + m_bonusSpeed; }
	/** 基礎速度を返す（表示用） */
	float GetBaseSpeed() const { return m_baseSpeed; }
	/** 上乗せ速度を返す（表示用） */
	float GetBonusSpeed() const { return m_bonusSpeed; }
	/** 今の速度レベル（1〜5） */
	int GetLevel() const;
	/** 今の Lv の中での進み具合（0〜1）。HUD の針に使う */
	float GetLevelRate() const;
	/** 上乗せを1秒あたり perSecond だけ増やす（マイナスで減らす）。毎フレーム呼ぶ */
	void AddBonusPerSecond(float perSecond);

private:
	static constexpr int MAX_LEVEL = 5;							///< スピードレベルの最大

	const SpeedParam& m_param = nsGameParam::Get().speed;		///< 調整用の値（Assets/param/speed.json）
	float m_baseSpeed = 0.0f;									///< 基礎速度
	float m_bonusSpeed = 0.0f;									///< 上乗せの速度
};
