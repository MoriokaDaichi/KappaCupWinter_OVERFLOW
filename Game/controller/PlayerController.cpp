#include "stdafx.h"
#include "controller/PlayerController.h"

#include "input/InputManager.h"

void PlayerController::MakeCommand(CharacterCommand& command) const
{
	if (m_input == nullptr)
	{
		// 入力がないときは、何もしないコマンドにする
		command = CharacterCommand();
		return;
	}

	command.moveDir = CalcMoveDirection();
	
	command.jumpTrigger = m_input->IsTrigger(EnInputAction::Jump);
	command.grappleTrigger = m_input->IsTrigger(EnInputAction::Grapple);
	command.grappleRelease = m_input->IsRelease(EnInputAction::Grapple);
	command.shootPress = m_input->IsPress(EnInputAction::Shoot);
	command.aimPress = m_input->IsPress(EnInputAction::Aim);

	command.aimOrigin = g_camera3D->GetPosition();
	command.aimDirection = g_camera3D->GetForward();
}

Vector3 PlayerController::CalcMoveDirection() const
{
	//スティックの入力
	const Vector2 stick = m_input->GetMove();

	//カメラの前向き・右向き（地面と平行にする）
	Vector3 forward = g_camera3D->GetForward();
	forward.y = 0.0f;
	forward.Normalize();
	Vector3 right = g_camera3D->GetRight();
	right.y = 0.0f;
	right.Normalize();

	//移動方向の決定
	Vector3 moveDir = forward * stick.y + right * stick.x;

	//キーボードだと斜めの長さが 0.7 くらいになるので、長さを 1 にそろえる（入力がないときは 0 のまま）
	if (moveDir.LengthSq() > MOVE_INPUT_THRESHOLD)
	{
		moveDir.Normalize();
	}
	return moveDir;
}
