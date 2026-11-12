#include "stdafx.h"
#include "actor/characterState/FlyState.h"

#include "actor/BattleCharacter.h"

void FlyState::Update()
{
	const auto& fly = m_owner->GetParam().fly;

	const Vector3 targetVelocity = m_owner->GetCommand().moveDir * fly.speed;
	Vector3 diff = targetVelocity - m_owner->GetMoveSpeed();
	const float maxChange = fly.accel * g_gameTime->GetFrameDeltaTime();

	// 急に向きを変えず、accel で少しずつ目標の速度に近づける
	if (diff.LengthSq() > maxChange * maxChange)
	{
		diff.Normalize();
		diff *= maxChange;
	}

	m_owner->SetMoveSpeed(m_owner->GetMoveSpeed() + diff);
}
