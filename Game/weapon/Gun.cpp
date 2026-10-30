#include "stdafx.h"
#include "weapon/Gun.h"

#include "weapon/BulletManager.h"
#include "collision/RayCast.h"
#include "actor/BattleCharacter.h"

void Gun::Update(const bool isShootPress, const bool isAimPress, const int level, const Vector3& chest, const Quaternion& rotation,
	const Vector3& aimOrigin, const Vector3& aimDirection)
{
	const int index = max(0, min(level - 1, static_cast<int>(m_param.levels.size()) - 1));
	const GunParam::Level& levelParam = m_param.levels[index];

	if (isAimPress && levelParam.lockCount > 0)
	{
		m_multiLocker.Update(aimOrigin, aimDirection, levelParam.lockCount);
	}

	if (m_wasAimPress && !isAimPress)
	{
		FireAtLocked(CalcMuzzle(chest, rotation), levelParam);
		m_multiLocker.Clear();
	}

	m_wasAimPress = isAimPress;

	if (isShootPress)
	{
		m_aimTimer = m_param.aimHoldTime;
	}

	m_aimTimer -= g_gameTime->GetFrameDeltaTime();

	m_shootTimer -= g_gameTime->GetFrameDeltaTime();
	if (!isShootPress || m_shootTimer > 0.0f)
	{
		return;
	}

	Shoot(CalcMuzzle(chest, rotation), levelParam);
	m_shootTimer = levelParam.shootInterval;
}

void Gun::Shoot(const Vector3& muzzle, const GunParam::Level& levelParam)
{
	if (m_bulletManager == nullptr)
	{
		return;
	}

	Vector3 dir = CalcAimPoint() - muzzle;
	if (dir.Length() < AIM_MIN_DISTANCE)
	{
		dir = g_camera3D->GetForward();
	}

	dir.Normalize();
	m_bulletManager->Fire(muzzle, dir * levelParam.bulletSpeed, levelParam);
}

Vector3 Gun::CalcAimPoint() const
{
	const Vector3 from = g_camera3D->GetPosition();
	const Vector3 to = from + g_camera3D->GetForward() * m_param.range;

	const RayHitInfo hit = nsCollision::RayCast(from, to);
	if (hit.isHit)
	{
		return hit.position;
	}

	return to;
}

Vector3 Gun::CalcMuzzle(const Vector3& chest, const Quaternion& rotation) const
{
	Vector3 right = Vector3::Right;
	rotation.Apply(right);
	Vector3 forward = Vector3::Front;
	rotation.Apply(forward);

	const Vector3 muzzle = chest + right * m_param.muzzleRight + forward * m_param.muzzleForward;

	// 壁にくっついていると銃口が壁の中に入る。中から飛ばしたレイは壁に当たらず弾が抜けるので、胸から出す
	if (nsCollision::RayCast(chest, muzzle).isHit)
	{
		return chest;
	}

	return muzzle;
}

void Gun::FireAtLocked(const Vector3& muzzle, const GunParam::Level& levelParam)
{
	if (m_bulletManager == nullptr)
	{
		return;
	}
	
	// 全部の弾を照準の方へ出し、飛びながらそれぞれの敵へ曲がっていく
	Vector3 dir = CalcAimPoint() - muzzle;
	if (dir.Length() < AIM_MIN_DISTANCE)
	{
		dir = g_camera3D->GetForward();
	}

	dir.Normalize();
	for (const BattleCharacter* target : m_multiLocker.GetTargets())
	{
		m_bulletManager->FireHoming(muzzle, dir * m_param.homingSpeed, levelParam, target);
	}
}
