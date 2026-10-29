#include "stdafx.h"
#include "controller/AIController.h"

#include "actor/Character.h"

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

	// 狙いは相手の方（足元から足元）。撃つのは敵のタスクで足す
	Vector3 toTargetAim = m_target->GetPosition() - m_owner->GetPosition();
	toTargetAim.Normalize();

	command.aimOrigin = m_owner->GetPosition();
	command.aimDirection = toTargetAim;
}
