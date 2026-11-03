#include "stdafx.h"
#include "weapon/BulletManager.h"
#include "collision/RayCast.h"
#include "actor/BattleCharacter.h"
#include "effect/EffectManager.h"
#include <algorithm>

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

	CollectEnemies();

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

		UpdateHoming(bullet, dt);

		const Vector3 nextPos = bullet.position + bullet.velocity * dt;
		// 今の位置から次の位置まで線で調べる（速いので、点で調べると薄い壁をすり抜ける）
		const RayHitInfo hit = nsCollision::RayCast(bullet.position, nextPos);

		// 壁の向こうの敵には当てない
		if (HitEnemies(bullet, hit.isHit ? hit.position : nextPos))
		{
			Deactivate(i);
			continue;
		}

		if (hit.isHit)
		{
			OnBulletImpact(bullet, hit.position);
			Deactivate(i);
			continue;
		}

		bullet.position = nextPos;
		UpdateBulletView(i);
	}
}

void BulletManager::Fire(const Vector3& position, const Vector3& velocity, const GunParam::Level& levelParam, BattleCharacter* shooter)
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
			bullet.damage = levelParam.damage;
			bullet.shooter.Reset(shooter);
			bullet.target.Clear();
			bullet.hitEnemyIds.clear();
			bullet.tracer.Reset(nsEffect::Play(EnEffect::Bullet, position, levelParam.bulletScale * m_effectParam.bulletScale));
			UpdateBulletView(i);

			return;
		}
	}
}

void BulletManager::FireHoming(const Vector3& position, const Vector3& velocity, const GunParam::Level& levelParam, BattleCharacter* shooter,
	const GOHandle<BattleCharacter>& target)
{
	for (int i = 0; i < MAX_BULLET; i++)
	{
		if (!m_bullets[i].isActive)
		{
			// Fire は最初に見つけた空きを使うので、同じ枠に入る
			Fire(position, velocity, levelParam, shooter);
			m_bullets[i].target = target;
			return;
		}
	}
}

void BulletManager::UpdateHoming(Bullet& bullet, const float dt) const
{
	const BattleCharacter* target = bullet.target.Get();
	if (target == nullptr || !target->CanBeTargeted())
	{
		return;
	}

	Vector3 toTarget = target->GetChestPosition() - bullet.position;
	const float reach = m_param.homingSpeed * dt;
	const bool isInReach = toTarget.LengthSq() <= reach * reach;

	// 今の向きから相手の向きへ、1 秒に homingTurnRate の割合で寄せる。このフレームで届くなら、外さないようにまっすぐ向ける
	toTarget.Normalize();
	Vector3 dir = toTarget;
	if (!isInReach)
	{
		dir = bullet.velocity;
		dir.Normalize();
		dir.Lerp(min(m_param.homingTurnRate * dt, 1.0f), dir, toTarget);
		dir.Normalize();
	}
	bullet.velocity = dir * m_param.homingSpeed;
}

void BulletManager::CollectEnemies()
{
	m_enemies.clear();
	QueryGOs<BattleCharacter>("enemy", [&](BattleCharacter* enemy)
		{
			m_enemies.push_back(enemy);
			return true;
		});
}

bool BulletManager::HitEnemies(Bullet& bullet, const Vector3& end)
{
	Vector3 hitPoint;
	BattleCharacter* enemy = FindFirstHit(bullet, end, hitPoint);
	// 当たった敵は hitEnemyIds に入って次は探さないので、このループは必ず終わる
	while (enemy != nullptr)
	{
		DamageEnemy(bullet, *enemy);
		OnBulletImpact(bullet, hitPoint);
		if (!bullet.pierce)
		{
			return true;
		}

		enemy = FindFirstHit(bullet, end, hitPoint);
	}

	return false;
}

BattleCharacter* BulletManager::FindFirstHit(const Bullet& bullet, const Vector3& end, Vector3& hitPoint) const
{
	const float bulletRadius = bullet.scale * MODEL_RADIUS;
	BattleCharacter* first = nullptr;
	float firstDistanceSq = 0.0f;

	for (BattleCharacter* enemy : m_enemies)
	{
		// 同じフレームのうちに倒された敵も入っているので、確かめる
		if (!enemy->CanBeTargeted() || HasHit(bullet, *enemy))
		{
			continue;
		}

		Vector3 point;
		if (!enemy->IsHitBySegment(bullet.position, end, bulletRadius, point))
		{
			continue;
		}

		const Vector3 fromBullet = point - bullet.position;
		const float distanceSq = fromBullet.LengthSq();
		if (first == nullptr || distanceSq < firstDistanceSq)
		{
			first = enemy;
			firstDistanceSq = distanceSq;
			hitPoint = point;
		}
	}

	return first;
}

bool BulletManager::HasHit(const Bullet& bullet, const BattleCharacter& enemy) const
{
	return std::find(bullet.hitEnemyIds.begin(), bullet.hitEnemyIds.end(), enemy.GetInstanceID()) != bullet.hitEnemyIds.end();
}

void BulletManager::DamageEnemy(Bullet& bullet, BattleCharacter& enemy)
{
	bullet.hitEnemyIds.push_back(enemy.GetInstanceID());

	// 貫通弾は相手を抜けたらまっすぐ飛ぶ（追い続けると、相手のまわりを回ってしまう）
	if (bullet.target.GetInstanceID() == enemy.GetInstanceID())
	{
		bullet.target.Clear();
	}

	// TakeDamage で隠れる・消えることがあるので、胸の位置は先に出しておく
	const Vector3 chest = enemy.GetChestPosition();
	if (!enemy.TakeDamage(bullet.damage))
	{
		return;
	}

	nsEffect::Play(EnEffect::Defeat, chest, m_effectParam.defeatScale);

	BattleCharacter* shooter = bullet.shooter.Get();
	if (shooter != nullptr)
	{
		shooter->GetSpeedSystem().AddBase(enemy.GetParam().health.killBoost);
	}
}

void BulletManager::OnBulletImpact(Bullet& bullet, const Vector3& position)
{
	if (bullet.explosionRadius > 0.0f)
	{
		Explode(bullet, position);
		return;
	}

	nsEffect::Play(EnEffect::Hit, position, m_effectParam.hitScale);
}

void BulletManager::Explode(Bullet& bullet, const Vector3& center)
{
	for (BattleCharacter* enemy : m_enemies)
	{
		if (!enemy->CanBeTargeted() || HasHit(bullet, *enemy))
		{
			continue;
		}

		Vector3 point;
		if (enemy->IsHitBySegment(center, center, bullet.explosionRadius, point))
		{
			DamageEnemy(bullet, *enemy);
		}
	}

	// エフェクトは半径 1 に広がるように作ってあるので、半径をそのまま拡大率にする
	nsEffect::Play(EnEffect::Explosion, center, bullet.explosionRadius);
}

void BulletManager::UpdateBulletView(const int index)
{
	const Bullet& bullet = m_bullets[index];

	EffectEmitter* tracer = bullet.tracer.Get();
	if (tracer == nullptr)
	{
		const float scale = bullet.scale;
		m_modelRender.UpdateInstancingData(index, bullet.position, Quaternion::Identity, Vector3(scale, scale, scale));
		return;
	}

	// 光の筋はエフェクトの上（Y 軸）に長いので、上を進む向きに合わせる
	Quaternion rotation;
	rotation.SetRotation(Vector3::Up, bullet.velocity);
	tracer->SetPosition(bullet.position);
	tracer->SetRotation(rotation);
}

void BulletManager::Deactivate(const int index)
{
	Bullet& bullet = m_bullets[index];
	bullet.isActive = false;

	EffectEmitter* tracer = bullet.tracer.Get();
	if (tracer != nullptr)
	{
		// 止めると、次のフレームに EffectEmitter が自分で消える
		tracer->Stop();
	}
	m_modelRender.RemoveInstance(index);
}

void BulletManager::Render(RenderContext& rc)
{
	m_modelRender.Draw(rc);
}
