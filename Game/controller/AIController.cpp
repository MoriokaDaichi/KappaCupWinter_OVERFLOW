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

	// 相手への横の向き（上下は見ない）
	Vector3 toTarget = m_target->GetPosition() - m_owner->GetPosition();
	toTarget.y = 0.0f;

	if (toTarget.LengthSq() > STOP_DISTANCE * STOP_DISTANCE)
	{
		toTarget.Normalize();
		command.moveDir = toTarget;
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
