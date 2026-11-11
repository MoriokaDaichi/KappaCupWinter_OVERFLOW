#include "stdafx.h"
#include "controller/ShootDecider.h"

#include "actor/BattleCharacter.h"
#include "collision/RayCast.h"

void ShootDecider::Init(const CharacterParam::Shooter& param)
{
	m_param = &param;
}

bool ShootDecider::Update(const BattleCharacter& owner, const BattleCharacter& target)
{
	if (m_param == nullptr)
	{
		return false;
	}

	m_timer -= g_gameTime->GetFrameDeltaTime();
	if (m_timer < 0.0f)
	{
		m_timer = 0.0f;
	}

	bool isShoot = false;

	switch (m_phase)
	{
	case EnShootPhase::Wait:
		if (CanSeeTarget(owner, target))
		{
			ChangePhase(EnShootPhase::Aim, m_param->aimTime);
		}
		break;

	case EnShootPhase::Aim:
		// 狙っているときに見失ったら Wait。隠れたらすぐ狙われなくなる方が分かりやすいため
		if (!CanSeeTarget(owner, target))
		{
			ChangePhase(EnShootPhase::Wait, 0.0f);
			break;
		}

		if (m_timer <= 0.0f)
		{
			ChangePhase(EnShootPhase::Cooldown, m_param->cooldownTime);
			isShoot = true;
		}

		break;

	case EnShootPhase::Cooldown:
		if (m_timer <= 0.0f)
		{
			ChangePhase(EnShootPhase::Wait, 0.0f);
		}

		break;
	}

	return isShoot;
}

void ShootDecider::Reset()
{
	ChangePhase(EnShootPhase::Wait, 0.0f);
}

bool ShootDecider::CanSeeTarget(const BattleCharacter& owner, const BattleCharacter& target) const
{
	const Vector3 muzzle = owner.GetMuzzlePosition();
	const Vector3 chest = target.GetChestPosition();
	const Vector3 toTarget = chest - muzzle;
	if (toTarget.LengthSq() > m_param->detectRange * m_param->detectRange)
	{
		return false;
	}

	const RayHitInfo hit = nsCollision::RayCast(muzzle, chest);
	return !hit.isHit;
}

Vector3 ShootDecider::CalcAimPoint(const BattleCharacter& owner, const BattleCharacter& target) const
{
	return target.GetChestPosition();
}

void ShootDecider::ChangePhase(const EnShootPhase phase, const float time)
{
	m_phase = phase;
	m_timer = time;
}
