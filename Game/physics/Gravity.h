#pragma once
#include "param/GameParam.h"

/** 重力処理。毎フレーム速度の y を減らす。プレイヤーと敵で共通に使う */
class Gravity
{
public:
	/** velocity の y を重力の分だけ減らす。落ちているときは強め、落下速度には上限あり */
	void Apply(Vector3& velocity) const;
	/** 重力の倍率を変える。1 で通常、0 で重力なし（ウォールランやスイング中に使う） */
	void SetScale(float scale)
	{
		m_scale = scale;
	}

private:
	const GravityParam& m_param = nsGameParam::Get().gravity;	///< 調整用の値（Assets/param/gravity.json）

	float m_scale = 1.0f;		///< 重力の倍率
};
