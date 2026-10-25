#include "stdafx.h"
#include "actor/playerState/PlayerWallRunState.h"

#include "actor/Player.h"

void PlayerWallRunState::Enter()
{
	m_runTime = 0.0f;
	m_owner->SetVerticalSpeed(0.0f);

	m_owner->SetGravityScale(m_param.gravityScale);
	m_owner->ResetJumpCount();
}

void PlayerWallRunState::Update()
{
	if (m_owner->IsJumpTrigger())
	{
		m_owner->WallJump();
		m_owner->ChangeState(EnPlayerState::Air);
		return;
	}

	if (m_owner->IsOnGround())
	{
		m_owner->ChangeState(EnPlayerState::Run);
		return;
	}

	if (!m_owner->FindWall())
	{
		m_owner->ChangeState(EnPlayerState::Air);
		return;
	}

	Move();
}

void PlayerWallRunState::Exit()
{
	m_owner->EndWallRun();
}

void PlayerWallRunState::Move()
{
	const Vector3& wallNormal = m_owner->GetWallNormal();

	Vector3 along = Cross(wallNormal, Vector3::Up);
	along.Normalize();

	if (along.Dot(m_owner->GetMoveSpeed()) < 0.0f)
	{
		along *= -1.0f;
	}

	// 壁走りは最低でも wallRun.speed。速度システムの値がそれより速ければ、その速さで走る
	const float runSpeed = max(m_param.speed, m_owner->GetSpeedSystem().GetSpeed());
	Vector3 horizontalSpeed = along * runSpeed + wallNormal * -m_param.stickSpeed;

	m_owner->SetHorizontalSpeed(horizontalSpeed);

	m_owner->FaceDirection(along);
	m_runTime += g_gameTime->GetFrameDeltaTime();
	if (m_runTime < m_param.maxTime)
	{
		m_owner->SetVerticalSpeed(0.0f);
	}
	else
	{
		m_owner->ApplyGravity();
	}
}
