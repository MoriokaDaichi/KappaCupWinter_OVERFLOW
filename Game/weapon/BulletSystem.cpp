#include "stdafx.h"
#include "weapon/BulletSystem.h"
#include "collision/RayCast.h"
#include "actor/BattleCharacter.h"
#include "effect/EffectManager.h"
#include <algorithm>

bool BulletSystem::Start()
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

void BulletSystem::Update()
{
	const float dt = g_gameTime->GetFrameDeltaTime();

	CollectTargets();

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

		// 壁の向こうの相手には当てない
		if (HitTargets(bullet, hit.isHit ? hit.position : nextPos))
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
		UpdateSplit(i, dt);
	}
}

void BulletSystem::Fire(const Vector3& position, const Vector3& velocity, const BulletShot& shot, BattleCharacter* shooter)
{
	for (int i = 0; i < MAX_BULLET; i++)
	{
		Bullet& bullet = m_bullets[i];

		if (!bullet.isActive)
		{
			bullet.isActive = true;
			bullet.position = position;
			bullet.velocity = velocity;
			bullet.lifeTimer = shot.lifeTime;
			bullet.scale = shot.scale;
			bullet.pierce = shot.pierce;
			bullet.explosionRadius = shot.explosionRadius;
			bullet.damage = shot.damage;
			bullet.shooter.Reset(shooter);
			bullet.target.Clear();
			bullet.hitTargetIds.clear();
			bullet.tracer.Reset(nsEffect::Play(shot.tracer, position, shot.scale * m_effectParam.bulletScale));
			bullet.splitTargets.clear();
			UpdateBulletView(i);

			return;
		}
	}
}

void BulletSystem::FireHoming(const Vector3& position, const Vector3& velocity, const BulletShot& shot, BattleCharacter* shooter,
	const GOHandle<BattleCharacter>& target)
{
	for (int i = 0; i < MAX_BULLET; i++)
	{
		if (!m_bullets[i].isActive)
		{
			// Fire は最初に見つけた空きを使うので、同じ枠に入る
			Fire(position, velocity, shot, shooter);
			m_bullets[i].target = target;
			return;
		}
	}
}

void BulletSystem::FireSplit(const Vector3& position, const Vector3& velocity, const BulletShot& shot, BattleCharacter* shooter,
	const std::vector<GOHandle<BattleCharacter>>& targets)
{
	for (int i = 0; i < MAX_BULLET; i++)
	{
		if (!m_bullets[i].isActive)
		{
			// Fire は最初に見つけた空きを使うので、同じ枠に入る
			Fire(position, velocity, shot, shooter);
			m_bullets[i].splitShot = shot;
			m_bullets[i].splitTimer = m_param.splitDelay;
			m_bullets[i].splitTargets = targets;
			return;
		}
	}
}

void BulletSystem::UpdateHoming(Bullet& bullet, const float dt) const
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

void BulletSystem::CollectTargets()
{
	m_hitTargets.clear();
	QueryGOs<BattleCharacter>(m_targetName, [&](BattleCharacter* hitTarget)
		{
			m_hitTargets.push_back(hitTarget);
			return true;
		});
}

bool BulletSystem::HitTargets(Bullet& bullet, const Vector3& end)
{
	Vector3 hitPoint;
	BattleCharacter* hitTarget = FindFirstHit(bullet, end, hitPoint);
	// 当たった相手は hitTargetIds に入って次は探さないので、このループは必ず終わる
	while (hitTarget != nullptr)
	{
		DamageTarget(bullet, *hitTarget);
		OnBulletImpact(bullet, hitPoint);
		if (!bullet.pierce)
		{
			return true;
		}

		hitTarget = FindFirstHit(bullet, end, hitPoint);
	}

	return false;
}

BattleCharacter* BulletSystem::FindFirstHit(const Bullet& bullet, const Vector3& end, Vector3& hitPoint) const
{
	const float bulletRadius = bullet.scale * MODEL_RADIUS;
	BattleCharacter* first = nullptr;
	float firstDistanceSq = 0.0f;

	for (BattleCharacter* hitTarget : m_hitTargets)
	{
		// 同じフレームのうちに倒された相手も入っているので、確かめる
		if (!hitTarget->CanBeTargeted() || HasHit(bullet, *hitTarget))
		{
			continue;
		}

		Vector3 point;
		if (!hitTarget->IsHitBySegment(bullet.position, end, bulletRadius, point))
		{
			continue;
		}

		const Vector3 fromBullet = point - bullet.position;
		const float distanceSq = fromBullet.LengthSq();
		if (first == nullptr || distanceSq < firstDistanceSq)
		{
			first = hitTarget;
			firstDistanceSq = distanceSq;
			hitPoint = point;
		}
	}

	return first;
}

bool BulletSystem::HasHit(const Bullet& bullet, const BattleCharacter& hitTarget) const
{
	return std::find(bullet.hitTargetIds.begin(), bullet.hitTargetIds.end(), hitTarget.GetInstanceID()) != bullet.hitTargetIds.end();
}

void BulletSystem::DamageTarget(Bullet& bullet, BattleCharacter& hitTarget)
{
	bullet.hitTargetIds.push_back(hitTarget.GetInstanceID());

	// 貫通弾は相手を抜けたらまっすぐ飛ぶ（追い続けると、相手のまわりを回ってしまう）
	if (bullet.target.GetInstanceID() == hitTarget.GetInstanceID())
	{
		bullet.target.Clear();
	}

	// TakeDamage で隠れる・消えることがあるので、胸の位置は先に出しておく
	const Vector3 chest = hitTarget.GetChestPosition();
	if (!hitTarget.TakeDamage(bullet.damage))
	{
		return;
	}

	nsEffect::Play(EnEffect::Defeat, chest, m_effectParam.defeatScale);

	BattleCharacter* shooter = bullet.shooter.Get();
	if (shooter != nullptr)
	{
		shooter->GetSpeedSystem().AddBase(hitTarget.GetParam().health.killBoost);
	}
}

void BulletSystem::OnBulletImpact(Bullet& bullet, const Vector3& position)
{
	if (bullet.explosionRadius > 0.0f)
	{
		Explode(bullet, position);
		return;
	}

	nsEffect::Play(EnEffect::Hit, position, m_effectParam.hitScale);
}

void BulletSystem::Explode(Bullet& bullet, const Vector3& center)
{
	for (BattleCharacter* hitTarget : m_hitTargets)
	{
		if (!hitTarget->CanBeTargeted() || HasHit(bullet, *hitTarget))
		{
			continue;
		}

		Vector3 point;
		if (hitTarget->IsHitBySegment(center, center, bullet.explosionRadius, point))
		{
			DamageTarget(bullet, *hitTarget);
		}
	}

	// エフェクトは半径 1 に広がるように作ってあるので、半径をそのまま拡大率にする
	nsEffect::Play(EnEffect::Explosion, center, bullet.explosionRadius);
}

void BulletSystem::UpdateBulletView(const int index)
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

void BulletSystem::Deactivate(const int index)
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

void BulletSystem::UpdateSplit(const int index, const float dt)
{
	Bullet& bullet = m_bullets[index];

	if (bullet.splitTargets.empty())
	{
		return;
	}

	bullet.splitTimer -= dt;
	if (bullet.splitTimer > 0.0f)
	{
		return;
	}

	// Deactivate で空いた枠に Fire が別の弾を入れるので、先に写しておく
	const Vector3 position = bullet.position;
	const Vector3 velocity = bullet.velocity;
	const BulletShot shot = bullet.splitShot;
	BattleCharacter* shooter = bullet.shooter.Get();
	const std::vector<GOHandle<BattleCharacter>> targets = bullet.splitTargets;

	Deactivate(index);

	for (const GOHandle<BattleCharacter>& target : targets)
	{
		FireHoming(position, velocity, shot, shooter, target);
	}
}

void BulletSystem::Render(RenderContext& rc)
{
	m_modelRender.Draw(rc);
}
