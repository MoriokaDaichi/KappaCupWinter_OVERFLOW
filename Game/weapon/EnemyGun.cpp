#include "stdafx.h"
#include "weapon/EnemyGun.h"

#include "weapon/BulletSystem.h"
#include "param/GameParam.h"

void EnemyGun::Init(BulletSystem* bulletSystem)
{
	m_bulletSystem = bulletSystem;
}

void EnemyGun::Update(const bool isShootPress, const Vector3& muzzle, const Vector3& aimDirection)
{
	if (!isShootPress || m_bulletSystem == nullptr)
	{
		return;
	}

	const EnemyBulletParam& param = nsGameParam::Get().enemyBullet;

	BulletShot shot;
	shot.tracer = EnEffect::EnemyBullet;
	shot.lifeTime = param.lifeTime;
	shot.scale = param.scale;
	shot.damage = param.damage;

	Vector3 dir = aimDirection;
	dir.Normalize();

	m_bulletSystem->Fire(muzzle, dir * param.speed, shot, nullptr);
}
