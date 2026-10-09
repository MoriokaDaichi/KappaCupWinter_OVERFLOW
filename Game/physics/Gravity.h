#pragma once

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
	static constexpr float GRAVITY = 980.0f;		///< 重力の強さ（1秒あたりに増える落下速度）
	static constexpr float FALL_MULTIPLIER = 1.5f;	///< 落ちているときに重力に掛ける倍率
	static constexpr float MAX_FALL_SPEED = 1500.0f;	///< 落下速度の上限

	float m_scale = 1.0f;		///< 重力の倍率
};
