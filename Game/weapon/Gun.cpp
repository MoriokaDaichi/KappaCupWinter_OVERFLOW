#include "stdafx.h"
#include "weapon/Gun.h"

#include "weapon/Bullet.h"
#include "collision/RayCast.h"

void Gun::Update(bool isShootPress, const Vector3& muzzle)
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

	Shoot(muzzle);
	m_shootTimer = m_param.shootInterval;
}

void Gun::Shoot(const Vector3& muzzle)
{
	Vector3 dir = CalcAimPoint() - muzzle;
	if (dir.Length() < AIM_MIN_DISTANCE)
	{
		dir = g_camera3D->GetForward();
	}

	dir.Normalize();
	Bullet* bullet = NewGO<Bullet>(0, "bullet");
	bullet->Init(muzzle, dir * m_param.bulletSpeed);
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
