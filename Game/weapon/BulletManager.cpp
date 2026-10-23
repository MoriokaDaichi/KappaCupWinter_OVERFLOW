#include "stdafx.h"
#include "weapon/BulletManager.h"
#include "collision/RayCast.h"

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

		Vector3 nextPos = bullet.position + bullet.velocity * dt;
		// 今の位置から次の位置まで線で調べる（速いので、点で調べると薄い壁をすり抜ける）
		RayHitInfo hit = nsCollision::RayCast(bullet.position, nextPos);
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

			return;
		}
	}
}

void BulletManager::Deactivate(int index)
{
	m_bullets[index].isActive = false;
	m_modelRender.RemoveInstance(index);
}

void BulletManager::Render(RenderContext& rc)
{
	m_modelRender.Draw(rc);
}
