#include "stdafx.h"
#include "actor/characterState/WallRunState.h"

#include "actor/BattleCharacter.h"
#include "collision/RayCast.h"

WallRunState::WallRunState(BattleCharacter* owner)
	: CharacterState(owner)
	, m_param(owner->GetParam().wallRun)
	, m_bodyParam(owner->GetParam().body)
{
}

void WallRunState::Enter()
{
	FindWall(*m_owner, m_wallNormal);
	m_runTime = 0.0f;
	m_owner->SetVerticalSpeed(0.0f);

	m_owner->SetGravityScale(m_param.gravityScale);
	m_owner->ResetJumpCount();
}

void WallRunState::Update()
{
	// 切り替え（壁ジャンプ・着地・壁がなくなった）は切り替え表が先に見る。ここに来たら壁はまだある（WallLost が成り立たなかった）ので、
	// 今の壁の向きを取り直してから走る。取れなかったときは前の向きのまま
	FindWall(*m_owner, m_wallNormal);
	Move();
}

void WallRunState::Exit()
{
	m_owner->SetGravityScale(1.0f);
	m_owner->StartCooldown(EnCooldown::WallRun, m_param.reattachTime);
}

void WallRunState::DoAction(const EnTransitionAction action)
{
	if (action == EnTransitionAction::WallJump)
	{
		WallJump();
	}
}

bool WallRunState::FindWall(const BattleCharacter& owner, Vector3& outNormal)
{
	const CharacterParam& param = owner.GetParam();

	Vector3 right = Vector3::Right;
	owner.GetRotation().Apply(right);

	Vector3 from = owner.GetPosition();
	from.y += param.body.chestHeight;

	const Vector3 dirs[] = { right, right * -1.0f };
	for (const Vector3& dir : dirs)
	{
		const Vector3 to = from + dir * param.wallRun.checkLength;
		const RayHitInfo hit = nsCollision::RayCast(from, to);

		if (!hit.isHit)
		{
			continue;
		}

		if (fabsf(hit.normal.y) >= param.wallRun.normalYLimit)
		{
			continue;
		}

		outNormal = hit.normal;
		return true;
	}

	return false;
}

void WallRunState::Move()
{
	Vector3 along = Cross(m_wallNormal, Vector3::Up);
	along.Normalize();

	if (along.Dot(m_owner->GetMoveSpeed()) < 0.0f)
	{
		along *= -1.0f;
	}

	// 壁走りは最低でも speed。速度システムの値がそれより速ければ、その速さで走る
	const float runSpeed = max(m_param.speed, m_owner->GetSpeedSystem().GetSpeed());
	const Vector3 horizontalSpeed = along * runSpeed + m_wallNormal * -m_param.stickSpeed;

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

	UpdateTilt(m_wallNormal);
}

void WallRunState::UpdateTilt(const Vector3& wallNormal)
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

void WallRunState::WallJump()
{
	m_owner->AddMoveSpeed(m_wallNormal * m_param.jumpSpeedOut);
	m_owner->SetVerticalSpeed(m_param.jumpSpeedUp);
	m_owner->AddJumpCount();
}
