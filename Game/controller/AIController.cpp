#include "stdafx.h"
#include "controller/AIController.h"

#include "actor/BattleCharacter.h"

void AIController::MakeCommand(CharacterCommand& command) const
{
	// 何もしないコマンドから始めて、必要なところだけ入れる
	command = CharacterCommand();

	if (m_owner == nullptr || m_target == nullptr)
	{
		return;
	}

	// 相手への横の向き（上下は見ない）
	Vector3 toTarget = m_target->GetPosition() - m_owner->GetPosition();
	toTarget.y = 0.0f;

	if (toTarget.LengthSq() > STOP_DISTANCE * STOP_DISTANCE)
	{
		toTarget.Normalize();
		command.moveDir = toTarget;
	}

	// 狙いは相手の方（胸から胸）。砲身もこの向きを見る
	Vector3 toTargetAim = m_target->GetChestPosition() - m_owner->GetChestPosition();
	toTargetAim.Normalize();

	command.aimOrigin = m_owner->GetChestPosition();
	command.aimDirection = toTargetAim;
}
