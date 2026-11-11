#include "stdafx.h"
#include "weapon/EnemyGun.h"

#include "weapon/BulletSystem.h"
#include "param/GameParam.h"

void EnemyGun::Init(BulletSystem* bulletSystem)
{
	m_bulletSystem = bulletSystem;
	m_warningLine.Init();
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

void EnemyGun::UpdateWarningLine(const bool isAimPress, const Vector3& muzzle, const Vector3& aimDirection, const float aimDistance)
{
	// 狙う点を少し過ぎた所で止める。弾が届く所まで伸ばすと、線がプレイヤーを通り抜けてカメラの横まで来て、手前が太い帯になる
	const EnemyBulletParam& bulletParam = nsGameParam::Get().enemyBullet;
	const float reach = bulletParam.speed * bulletParam.lifeTime;
	const float length = aimDistance > 0.0f ? min(aimDistance + nsGameParam::Get().warningLine.pastTargetLength, reach) : reach;
	m_warningLine.Update(isAimPress, muzzle, aimDirection, length);
}

void EnemyGun::Draw(RenderContext& rc)
{
	m_warningLine.Draw(rc);
}
