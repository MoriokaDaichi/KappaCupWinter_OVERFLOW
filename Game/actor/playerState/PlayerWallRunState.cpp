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

	UpdateTilt(wallNormal);
}

void PlayerWallRunState::UpdateTilt(const Vector3& wallNormal)
{
	// 体の上を壁から離れる向きへ傾け、足を壁につける。壁の向きから決めるので、左右どちらの壁でも同じ
	Vector3 away = wallNormal;
	away.y = 0.0f;
	away.Normalize();
	const float angle = Math::DegToRad(m_param.tiltAngle);
	Quaternion target;
	target.SetRotation(Vector3::Up, Vector3::Up * cosf(angle) + away * sinf(angle));

	// 回す中心を「傾けたときに足が壁（カプセルの端）に届く高さ」にする
	const float pivotHeight = m_bodyParam.radius / max(sinf(angle), TILT_MIN_SIN);

	m_owner->SetModelTiltTarget(target, pivotHeight);
}
