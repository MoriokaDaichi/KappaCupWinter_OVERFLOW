#include "stdafx.h"
#include "weapon/BulletManager.h"
#include "collision/RayCast.h"
#include "actor/BattleCharacter.h"

bool BulletManager::Start()
{
	// 弾はすぐ消えるので、レイトレには入れない（入れると作るたび・消すたびに重くなる）
	m_modelRender.SetRaytracingWorld(false);
	m_modelRender.Init("Assets/modelData/debug/debug_anchor.tkm", nullptr, 0, enModelUpAxisZ, true, MAX_BULLET);

	for (int i = 0; i < MAX_BULLET; i++)
	{
		// インスタンシングは空きの枠も毎回描くので、最初に全部消しておく（消さないと原点に弾が並ぶ）
		m_modelRender.RemoveInstance(i);
	}

	return true;
}

void BulletManager::Update()
{
	const float dt = g_gameTime->GetFrameDeltaTime();

	for (int i = 0; i < MAX_BULLET; i++)
	{
		Bullet& bullet = m_bullets[i];

		if (!bullet.isActive)
		{
			continue;
		}

		bullet.lifeTimer -= dt;
		if (bullet.lifeTimer <= 0.0f)
		{
			Deactivate(i);
			continue;
		}

		if (bullet.target != nullptr && UpdateHoming(bullet, dt))
		{
			Deactivate(i);
			continue;
		}

		const Vector3 nextPos = bullet.position + bullet.velocity * dt;
		// 今の位置から次の位置まで線で調べる（速いので、点で調べると薄い壁をすり抜ける）
		const RayHitInfo hit = nsCollision::RayCast(bullet.position, nextPos);
		if (hit.isHit)
		{
			Deactivate(i);
			continue;
		}

		bullet.position = nextPos;
		const float scale = bullet.scale;
		m_modelRender.UpdateInstancingData(i, bullet.position, Quaternion::Identity, Vector3(scale, scale, scale));
	}
}

void BulletManager::Fire(const Vector3& position, const Vector3& velocity, const GunParam::Level& levelParam)
{
	for (int i = 0; i < MAX_BULLET; i++)
	{
		Bullet& bullet = m_bullets[i];

		if (!bullet.isActive)
		{
			bullet.isActive = true;
			bullet.position = position;
			bullet.velocity = velocity;
			bullet.lifeTimer = m_param.bulletLifeTime;
			bullet.scale = levelParam.bulletScale;
			bullet.pierce = levelParam.pierce;
			bullet.explosionRadius = levelParam.explosionRadius;
			bullet.target = nullptr;

			return;
		}
	}
}

void BulletManager::FireHoming(const Vector3& position, const Vector3& velocity, const GunParam::Level& levelParam,
	const BattleCharacter* target)
{
	for (int i = 0; i < MAX_BULLET; i++)
	{
		if (!m_bullets[i].isActive)
		{
			// Fire は最初に見つけた空きを使うので、同じ枠に入る
			Fire(position, velocity, levelParam);
			m_bullets[i].target = target;
			return;
		}
	}
}

bool BulletManager::UpdateHoming(Bullet& bullet, const float dt) const
{
	Vector3 toTarget = bullet.target->GetChestPosition() - bullet.position;
	const float distance = toTarget.Length();
	// このフレームで届く（今は当たり判定がないので、届いたら消す。当てるのは HP のタスクで）
	if (distance <= m_param.homingSpeed * dt)
	{
		return true;
	}

	// 今の向きから相手の向きへ、1 秒に homingTurnRate の割合で寄せる
	toTarget.Normalize();
	Vector3 dir = bullet.velocity;
	dir.Normalize();
	dir.Lerp(min(m_param.homingTurnRate * dt, 1.0f), dir, toTarget);
	dir.Normalize();
	bullet.velocity = dir * m_param.homingSpeed;

	return false;
}

void BulletManager::Deactivate(const int index)
{
	m_bullets[index].isActive = false;
	m_modelRender.RemoveInstance(index);
}

void BulletManager::Render(RenderContext& rc)
{
	m_modelRender.Draw(rc);
}
