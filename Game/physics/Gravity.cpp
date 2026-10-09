#include "stdafx.h"
#include "physics/Gravity.h"

void Gravity::Apply(Vector3& velocity) const
{
	float gravity = GRAVITY * m_scale;		// 今の重力（倍率が 0 なら重力なし）
	
	if (velocity.y < 0.0f)
	{
		gravity *= FALL_MULTIPLIER;		// 落ちているときは強めて、キビキビ落とす
	}

	velocity.y -= gravity * g_gameTime->GetFrameDeltaTime();		// 1フレーム分だけ落下速度を増やす
	if (velocity.y < -MAX_FALL_SPEED)
	{
		velocity.y = -MAX_FALL_SPEED;		// 落下速度が上限を超えないようにする
	}
}
