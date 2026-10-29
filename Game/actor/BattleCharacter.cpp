#include "stdafx.h"
#include "actor/BattleCharacter.h"
#include "controller/IController.h"
#include "weapon/BulletManager.h"

bool BattleCharacter::IsJumpTrigger() const
{
	return m_command.jumpTrigger;
}

bool BattleCharacter::IsMoveInput() const
{
	return m_command.moveDir.LengthSq() > MOVE_INPUT_THRESHOLD;
}

Vector3 BattleCharacter::GetMoveDirection() const
{
	return m_command.moveDir;
}

bool BattleCharacter::IsGrappleTrigger() const
{
	return m_command.grappleTrigger;
}

bool BattleCharacter::IsGrappleRelease() const
{
	return m_command.grappleRelease;
}

bool BattleCharacter::IsShootPress() const
{
	return m_command.shootPress;
}

bool BattleCharacter::IsAimPress() const
{
	return m_command.aimPress;
}

void BattleCharacter::StartCooldown(const EnCooldown id, const float seconds)
{
	m_cooldownTimers[static_cast<int>(id)] = seconds;
}

bool BattleCharacter::IsCooldownOver(const EnCooldown id) const
{
	return m_cooldownTimers[static_cast<int>(id)] <= 0.0f;
}

void BattleCharacter::Move()
{
	const Vector3 moveDir = GetMoveDirection();

	//速度に入れる
	ApproachHorizontal(moveDir * GetSpeedSystem().GetSpeed(), m_param.move.groundAccel);

	//スティックを倒しているときだけ向きを変える（離したときは今の向きを保つ）
	FaceDirection(moveDir);
}

void BattleCharacter::AirMove()
{
	if (!IsMoveInput())
	{
		return;
	}

	const Vector3 moveDir = GetMoveDirection();
	const Vector3 target = moveDir * GetSpeedSystem().GetSpeed();

	float t = m_param.move.airControl * g_gameTime->GetFrameDeltaTime();
	if (t > 1.0f)
	{
		t = 1.0f;
	}

	//今の速さを目標の速さに t の割合だけ近づける
	m_moveSpeed.x += (target.x - m_moveSpeed.x) * t;
	m_moveSpeed.z += (target.z - m_moveSpeed.z) * t;

	FaceDirection(moveDir);
}

void BattleCharacter::Decelerate()
{
	ApproachHorizontal(Vector3::Zero, m_param.move.groundDecel);
}

void BattleCharacter::Jump()
{
	// = で入れるので、落ちている途中に跳んでも同じ高さだけ上がる
	SetVerticalSpeed(m_param.jump.speed);
	m_jumpCount += 1;
	// ジャンプしたことを子クラスに知らせる
	OnJump();
}

void BattleCharacter::ResetJumpCount()
{
	m_jumpCount = 0;
}

void BattleCharacter::AddJumpCount()
{
	m_jumpCount += 1;
}

void BattleCharacter::ConsumeGroundJump()
{
	if (m_jumpCount == 0)
	{
		m_jumpCount = 1;
	}
}

bool BattleCharacter::CanStartWallRun() const
{
	Vector3 horizontal = GetMoveSpeed();
	horizontal.y = 0.0f;
	const float speed = horizontal.Length();

	return IsMoveInput() && speed >= m_param.wallRun.minSpeed && IsCooldownOver(EnCooldown::WallRun);
}

Vector3 BattleCharacter::GetChestPosition() const
{
	Vector3 position = GetPosition();
	position.y += m_param.body.chestHeight;

	return position;
}

void BattleCharacter::UpdateCommand()
{
	if (m_controller != nullptr)
	{
		m_controller->MakeCommand(m_command);
	}

	UpdateCooldown();
}

void BattleCharacter::UpdateCooldown()
{
	for (float& timer : m_cooldownTimers)
	{
		timer -= g_gameTime->GetFrameDeltaTime();
		if (timer < 0.0f)
		{
			timer = 0.0f;
		}
	}
}

void BattleCharacter::InitAbilities()
{
	m_speed.Reset();
	m_grapple.Init();
	m_gun.Init(FindGO<BulletManager>("bulletManager"));
}
