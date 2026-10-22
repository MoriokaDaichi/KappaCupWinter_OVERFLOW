#include "stdafx.h"
#include "weapon/Gun.h"

#include "weapon/BulletManager.h"
#include "collision/RayCast.h"

void Gun::Update(bool isShootPress, const Vector3& chest, const Quaternion& rotation)
{
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

	Shoot(CalcMuzzle(chest, rotation));
	m_shootTimer = m_param.shootInterval;
}

void Gun::Shoot(const Vector3& muzzle)
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
	m_bulletManager->Fire(muzzle, dir * m_param.bulletSpeed);
}

Vector3 Gun::CalcAimPoint() const
{
	Vector3 from = g_camera3D->GetPosition();
	Vector3 to = from + g_camera3D->GetForward() * m_param.range;

	RayHitInfo hit = nsCollision::RayCast(from, to);
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

	Vector3 muzzle = chest + right * m_param.muzzleRight + forward * m_param.muzzleForward;

	// 壁にくっついていると銃口が壁の中に入る。中から飛ばしたレイは壁に当たらず弾が抜けるので、胸から出す
	if (nsCollision::RayCast(chest, muzzle).isHit)
	{
		return chest;
	}

	return muzzle;
}
