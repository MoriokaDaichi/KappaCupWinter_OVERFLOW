#include "stdafx.h"
#include "weapon/MultiLocker.h"

#include "actor/BattleCharacter.h"
#include "collision/RayCast.h"
#include <algorithm>

void MultiLocker::Update(const Vector3& aimOrigin, const Vector3& aimDirection, const int maxCount)
{
	// 1 秒で lockTime 分の 1 だけ満ちる
	const float step = g_gameTime->GetFrameDeltaTime() / max(m_param.lockTime, MIN_LOCK_TIME);
	const bool isFull = static_cast<int>(m_targets.size()) >= maxCount;

	// 照準に入っている敵を集める（もう満杯なら集めないので、ロックしかけの敵は全部減っていく）
	std::vector<BattleCharacter*> inAim;
	if (!isFull)
	{
		QueryGOs<BattleCharacter>("enemy", [&](BattleCharacter* enemy)
			{
				if (!IsLocked(enemy) && CanLock(*enemy, aimOrigin, aimDirection))
				{
					inAim.push_back(enemy);
				}

				return true;
			});
	}

	// 外れた敵は減らす
	for (Candidate& candidate : m_candidates)
	{
		if (std::find(inAim.begin(), inAim.end(), candidate.target) == inAim.end())
		{
			candidate.progress -= step;
		}
	}

	// 入っている敵は増やす（初めて入った敵は、ロックしかけに足す）
	for (BattleCharacter* enemy : inAim)
	{
		Candidate* candidate = FindCandidate(enemy);
		if (candidate == nullptr)
		{
			m_candidates.push_back({ enemy, 0.0f });
			candidate = &m_candidates.back();
		}

		candidate->progress += step;
	}

	// 満ちた敵をロックし、0 まで減った敵を外す
	for (auto it = m_candidates.begin(); it != m_candidates.end();)
	{
		if (it->progress >= 1.0f && static_cast<int>(m_targets.size()) < maxCount)
		{
			m_targets.push_back(it->target);
			it = m_candidates.erase(it);
		}
		else if (it->progress <= 0.0f)
		{
			it = m_candidates.erase(it);
		}
		else
		{
			// 満杯で入れられなかった敵は、1 のまま止めておく
			it->progress = min(it->progress, 1.0f);
			++it;
		}
	}
}

void MultiLocker::Clear()
{
	m_targets.clear();
	m_candidates.clear();
}

bool MultiLocker::CanLock(const BattleCharacter& target, const Vector3& aimOrigin, const Vector3& aimDirection) const
{
	const Vector3 targetPos = target.GetChestPosition();
	Vector3 toTarget = targetPos - aimOrigin;

	// 遠すぎる
	if (toTarget.Length() > m_param.lockDistance)
	{
		return false;
	}

	// 照準からの角度が大きすぎる（toTarget を正規化して aimDirection と内積）
	toTarget.Normalize();
	const float dot = aimDirection.Dot(toTarget);
	if (dot < cosf(Math::DegToRad(m_param.lockAngle)))
	{
		return false;
	}

	// 壁にさえぎられている（aimOrigin から targetPos へレイ）
	if (nsCollision::RayCast(aimOrigin, targetPos).isHit)
	{
		return false;
	}

	return true;
}

bool MultiLocker::IsLocked(const BattleCharacter* target) const
{
	return std::find(m_targets.begin(), m_targets.end(), target) != m_targets.end();
}

MultiLocker::Candidate* MultiLocker::FindCandidate(const BattleCharacter* target)
{
	for (Candidate& candidate : m_candidates)
	{
		if (candidate.target == target)
		{
			return &candidate;
		}
	}

	return nullptr;
}
