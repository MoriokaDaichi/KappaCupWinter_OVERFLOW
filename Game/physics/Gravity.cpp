#include "stdafx.h"
#include "physics/Gravity.h"

void Gravity::Apply(Vector3& velocity) const
{
	float gravity = m_param.gravity * m_scale;		// 今の重力（倍率が 0 なら重力なし）
	
	if (velocity.y < 0.0f)
	{
		gravity *= m_param.fallMultiplier;		// 落ちているときは強めて、キビキビ落とす
	}

	velocity.y -= gravity * g_gameTime->GetFrameDeltaTime();		// 1フレーム分だけ落下速度を増やす
	if (velocity.y < -m_param.maxFallSpeed)
	{
		velocity.y = -m_param.maxFallSpeed;		// 落下速度が上限を超えないようにする
	}
}
