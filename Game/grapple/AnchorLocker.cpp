#include "stdafx.h"
#include "grapple/AnchorLocker.h"

#include "gimmick/Anchor.h"
#include "collision/RayCast.h"

void AnchorLocker::Update(const Vector3& position, const Vector3& chestPosition, const Vector3& aimOrigin, const Vector3& aimDirection)
{
	Anchor* best = nullptr;
	float bestScore = 0.0f;

	QueryGOs<Anchor>("anchor", [&](Anchor* anchor)
		{
			float score = CalcLockScore(position, chestPosition, anchor, aimOrigin, aimDirection);

			if (score < 0.0f)
			{
				return true;
			}

			if (anchor == m_lockedAnchor)
			{
				score += m_param.keepBonus;
			}

			if (best == nullptr || score > bestScore)
			{
				best = anchor;
				bestScore = score;
			}

			return true;
		});

	if (best == m_lockedAnchor)
	{
		return;
	}

	if (m_lockedAnchor != nullptr)
	{
		m_lockedAnchor->SetLocked(false);
	}

	if (best != nullptr)
	{
		best->SetLocked(true);
	}

	m_lockedAnchor = best;
}

float AnchorLocker::CalcLockScore(const Vector3& position, const Vector3& chestPosition, const Anchor* anchor, const Vector3& aimOrigin, const Vector3& aimDirection) const
{
	const Vector3 toAnchor = anchor->GetPosition() - position;
	const float distance = toAnchor.Length();
	if (distance > m_param.maxDistance)
	{
		return -1.0f;
	}

	Vector3 camToAnchor = anchor->GetPosition() - aimOrigin;
	camToAnchor.Normalize();
	const float dot = aimDirection.Dot(camToAnchor);
	if (dot < cosf(Math::DegToRad(m_param.maxAngle)))
	{
		return -1.0f;
	}

	const float angle = Math::RadToDeg(acosf(min(dot, 1.0f)));

	if (nsCollision::RayCast(chestPosition, anchor->GetPosition()).isHit)
	{
		return -1.0f;
	}

	const float angleScore = 1.0f - angle / m_param.maxAngle;
	const float distanceScore = 1.0f - distance / m_param.maxDistance;

	return angleScore * m_param.angleWeight + distanceScore * m_param.distanceWeight;
}
