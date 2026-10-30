#include "stdafx.h"
#include "hud/LockOnHud.h"

#include "actor/BattleCharacter.h"

namespace
{
	// Vector4 は constexpr にできないので、クラスの定数ではなくここに置く
	const Vector4 FILLING_COLOR(1.0f, 1.0f, 1.0f, 0.9f);		///< ロックしかけの色
	const Vector4 LOCKED_COLOR(1.0f, 0.25f, 0.2f, 1.0f);		///< ロックした色
}

void LockOnHud::Init(const BattleCharacter* player)
{
	m_player = player;

	for (LockOnRing& ring : m_rings)
	{
		ring.Init(RING_SIZE);
	}
}

void LockOnHud::Draw(RenderContext& rc)
{
	if (m_player == nullptr || !m_player->HasGun())
	{
		return;
	}

	m_usedRingNum = 0;
	const MultiLocker& locker = m_player->GetGun().GetMultiLocker();

	for (const BattleCharacter* target : locker.GetTargets())
	{
		DrawRing(rc, *target, 1.0f, LOCKED_COLOR);
	}

	for (const MultiLocker::Candidate& candidate : locker.GetCandidates())
	{
		DrawRing(rc, *candidate.target, candidate.progress, FILLING_COLOR);
	}
}

bool LockOnHud::CalcScreenPosition(const BattleCharacter& target, Vector2& screenPos) const
{
	const Vector3 chest = target.GetChestPosition();

	// カメラの後ろの点も画面に映ってしまう（前後が逆さに出る）ので、出さない
	const Vector3 toTarget = chest - g_camera3D->GetPosition();
	if (toTarget.Dot(g_camera3D->GetForward()) <= 0.0f)
	{
		return false;
	}

	g_camera3D->CalcScreenPositionFromWorldPosition(screenPos, chest);
	return true;
}

void LockOnHud::DrawRing(RenderContext& rc, const BattleCharacter& target, const float progress, const Vector4& color)
{
	if (m_usedRingNum >= MAX_RING)
	{
		return;
	}

	Vector2 screenPos;
	if (!CalcScreenPosition(target, screenPos))
	{
		return;
	}

	LockOnRing& ring = m_rings[m_usedRingNum];
	ring.Update(screenPos, progress, color);
	ring.Draw(rc);
	m_usedRingNum++;
}
