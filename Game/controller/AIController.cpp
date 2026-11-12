#include "stdafx.h"
#include "controller/AIController.h"

#include "actor/BattleCharacter.h"

void AIController::MakeCommand(CharacterCommand& command)
{
	// 何もしないコマンドから始めて、必要なところだけ入れる
	command = CharacterCommand();

	if (m_owner == nullptr || m_target == nullptr)
	{
		return;
	}

	if (m_hasPatrol)
	{
		// 往復の端への向き。飛ぶ敵なので上下も見る
		Vector3 toGoal = m_patrolPoints[m_patrolIndex] - m_owner->GetPosition();
		if (toGoal.LengthSq() < PATROL_ARRIVE_DISTANCE * PATROL_ARRIVE_DISTANCE)
		{
			m_patrolIndex = 1 - m_patrolIndex;
			toGoal = m_patrolPoints[m_patrolIndex] - m_owner->GetPosition();
		}
		toGoal.Normalize();
		command.moveDir = toGoal;
	}
	else
	{
		// 相手への横の向き（上下は見ない）
		Vector3 toTarget = m_target->GetPosition() - m_owner->GetPosition();
		toTarget.y = 0.0f;

		if (toTarget.LengthSq() > STOP_DISTANCE * STOP_DISTANCE)
		{
			toTarget.Normalize();
			command.moveDir = toTarget;
		}
	}

	// 狙いは相手の方（砲口から狙う点へ）。砲身もこの向きを見る
	const Vector3 muzzle = m_owner->GetMuzzlePosition();
	const Vector3 aimPoint = m_shootDecider.CalcAimPoint(*m_owner, *m_target);

	const Vector3 toAimPoint = aimPoint - muzzle;
	Vector3 aimDirection = toAimPoint;
	aimDirection.Normalize();

	command.aimOrigin = muzzle;
	command.aimDirection = aimDirection;
	command.aimDistance = toAimPoint.Length();

	command.shootPress = m_shootDecider.Update(*m_owner, *m_target);
	// 敵の「構える」は狙っている間。段階を進めた後に聞くので、撃つフレームには線が消え、弾に入れ替わる
	command.aimPress = m_shootDecider.IsAiming();
}

void AIController::SetPatrol(const Vector3& start, const Vector3& end)
{
	m_patrolPoints[0] = start;
	m_patrolPoints[1] = end;
	m_hasPatrol = true;
	m_patrolIndex = 1;
}
