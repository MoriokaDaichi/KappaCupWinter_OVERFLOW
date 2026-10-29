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

	// 狙いはまだ使わない。向きを 0 にしておくと、アンカーをロックしない（ロック中の印はアンカーが持つので、プレイヤーの印を消さないように）
	command.aimOrigin = m_owner->GetPosition();
	command.aimDirection = Vector3::Zero;
}
