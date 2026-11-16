#include "stdafx.h"
#include "hud/LockOnHud.h"

#include "actor/BattleCharacter.h"
#include "camera/ScreenPosition.h"
#include "param/GameParam.h"

namespace
{
	// Vector4 は constexpr にできないので、クラスの定数ではなくここに置く
	const Vector4 FILLING_COLOR(1.0f, 1.0f, 1.0f, 0.9f);		///< ロックしかけの色
	const Vector4 LOCKED_COLOR(1.0f, 0.25f, 0.2f, 1.0f);		///< ロックした色
	const Vector4 LOCK_BOX_COLOR(0.0f, 0.90f, 1.0f, 0.5f);		///< ロックする範囲の四角の色（シアン #00e5ff を薄く）
}

void LockOnHud::Init(const BattleCharacter* player)
{
	m_player = player;

	for (LockOnRing& ring : m_rings)
	{
		ring.Init(RING_SIZE);
	}

	// 板は 1 枚だけ作るので、どの Lv の四角も入るように、いちばん大きい幅と高さにする
	for (const GunParam::Level& level : nsGameParam::Get().gun.levels)
	{
		m_lockBoxMaxSize.x = max(m_lockBoxMaxSize.x, level.lockBoxWidth);
		m_lockBoxMaxSize.y = max(m_lockBoxMaxSize.y, level.lockBoxHeight);
	}
	m_lockBox.Init("Assets/shader/hudLockBox.fx", m_lockBoxMaxSize.x, m_lockBoxMaxSize.y);
}

void LockOnHud::DrawLockBox()
{
	if (m_lockBoxMaxSize.x <= 0.0f || m_lockBoxMaxSize.y <= 0.0f)
	{
		return;
	}

	// 板に対する今の四角の割合をシェーダーに渡す（半分の大きさ × 2 ＝ 幅・高さ）
	const Vector2& halfSize = m_player->GetGun().GetLockBoxHalfSize();
	const float widthRate = halfSize.x * 2.0f / m_lockBoxMaxSize.x;
	const float heightRate = halfSize.y * 2.0f / m_lockBoxMaxSize.y;

	m_lockBox.Update(Vector2::Zero, LOCK_BOX_COLOR, widthRate, 0.0f, heightRate);
	m_lockBox.Draw();
}

void LockOnHud::Draw(RenderContext& rc)
{
	if (m_player == nullptr || !m_player->HasGun())
	{
		return;
	}

	DrawLockBox();

	m_usedRingNum = 0;
	const MultiLocker& locker = m_player->GetGun().GetMultiLocker();

	// このフレームに倒された敵は描かない（消えた敵は Get が nullptr を返し、隠れている敵は CanBeTargeted が false を返す）
	for (const GOHandle<BattleCharacter>& handle : locker.GetTargets())
	{
		const BattleCharacter* target = handle.Get();
		if (target != nullptr && target->CanBeTargeted())
		{
			DrawRing(rc, *target, 1.0f, LOCKED_COLOR);
		}
	}

	for (const MultiLocker::Candidate& candidate : locker.GetCandidates())
	{
		const BattleCharacter* target = candidate.target.Get();
		if (target != nullptr && target->CanBeTargeted())
		{
			DrawRing(rc, *target, candidate.progress, FILLING_COLOR);
		}
	}
}

void LockOnHud::DrawRing(RenderContext& rc, const BattleCharacter& target, const float progress, const Vector4& color)
{
	if (m_usedRingNum >= MAX_RING)
	{
		return;
	}

	Vector2 screenPos;
	if (!nsScreen::CalcScreenPosition(target.GetChestPosition(), screenPos))
	{
		return;
	}

	LockOnRing& ring = m_rings[m_usedRingNum];
	ring.Update(screenPos, progress, color);
	ring.Draw(rc);
	m_usedRingNum++;
}
