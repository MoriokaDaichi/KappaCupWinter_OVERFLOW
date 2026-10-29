#include "stdafx.h"
#include "actor/BattleCharacter.h"

#include "controller/IController.h"
#include "weapon/BulletManager.h"
#include "actor/playerState/PlayerIdleState.h"
#include "actor/playerState/PlayerRunState.h"
#include "actor/playerState/PlayerAirState.h"
#include "actor/playerState/PlayerWallRunState.h"
#include "actor/playerState/PlayerSwingState.h"
#include <algorithm>

BattleCharacter::BattleCharacter() : m_stateMachine(this)
{
}

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
	ApproachHorizontal(moveDir * GetSpeedSystem().GetSpeed(), m_param->move.groundAccel);

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

	float t = m_param->move.airControl * g_gameTime->GetFrameDeltaTime();
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
	ApproachHorizontal(Vector3::Zero, m_param->move.groundDecel);
}

void BattleCharacter::Jump()
{
	// = で入れるので、落ちている途中に跳んでも同じ高さだけ上がる
	SetVerticalSpeed(m_param->jump.speed);
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

	return IsMoveInput() && speed >= m_param->wallRun.minSpeed && IsCooldownOver(EnCooldown::WallRun);
}

Vector3 BattleCharacter::GetChestPosition() const
{
	Vector3 position = GetPosition();
	position.y += m_param->body.chestHeight;

	return position;
}

void BattleCharacter::ChangeState(const EnPlayerState id)
{
	m_stateMachine.ChangeState(id);
	OnStateChanged(id);
}

void BattleCharacter::InitState()
{
	for (const std::string& name : m_param->states)
	{
		EnPlayerState id = EnPlayerState::Num;
		const bool found = ToPlayerState(name, id);

		K2_ASSERT(found, "知らない State の名前です");

		AddStateById(id);
	}

	ChangeState(EnPlayerState::Idle);
}

void BattleCharacter::UpdateState()
{
	m_stateMachine.Update();
}

void BattleCharacter::UpdateAction()
{
	UpdateCommand();
	if (HasGrapple())
	{
		GetGrapple().UpdateLock(GetPosition(), GetChestPosition(), GetCommand().aimOrigin, GetCommand().aimDirection);
	}

	if (IsGrappleTrigger() && CanStartSwing())
	{
		ChangeState(EnPlayerState::Swing);
	}

	UpdateState();
	OnStateUpdated();
	UpdateSpeedBonus();
	// 銃口は体の向きから決めるので、先に体を向けてから撃つ
	FaceAimDirection();
	if (HasGun())
	{
		GetGun().Update(IsShootPress(), GetSpeedSystem().GetLevel(), GetChestPosition(), m_rotation);
	}
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

void BattleCharacter::FaceAimDirection()
{
	// 押した最初のフレームは、まだ Gun の構えのタイマーが入っていないので、ボタンも見る
	const bool isAiming = IsShootPress() || (HasGun() && GetGun().IsAiming()) || IsAimPress();
	if (!isAiming || GetCurrentState() == EnPlayerState::WallRun)
	{
		return;
	}

	FaceDirection(GetCommand().aimDirection);
}

void BattleCharacter::UpdateSpeedBonus()
{
	switch (GetCurrentState())
	{
	case EnPlayerState::WallRun:
		GetSpeedSystem().AddBonusPerSecond(m_speedParam.wallRunGain);
		break;
	case EnPlayerState::Run:
		GetSpeedSystem().AddBonusPerSecond(-m_speedParam.groundDecay);
		break;
	case EnPlayerState::Idle:
		GetSpeedSystem().AddBonusPerSecond(-m_speedParam.idleDecay);
		break;
	default:
		break;
	}
}

bool BattleCharacter::HasAbility(const char* name) const
{
	const std::vector<std::string>& abilities = m_param->abilities;
	return std::find(abilities.begin(), abilities.end(), name) != abilities.end();
}

void BattleCharacter::AddStateById(const EnPlayerState id)
{
	switch (id)
	{
	case EnPlayerState::Idle:
		m_stateMachine.AddState<PlayerIdleState>(id);
		break;
	case EnPlayerState::Run:
		m_stateMachine.AddState<PlayerRunState>(id);
		break;
	case EnPlayerState::Air:
		m_stateMachine.AddState<PlayerAirState>(id);
		break;
	case EnPlayerState::WallRun:
		m_stateMachine.AddState<PlayerWallRunState>(id);
		break;
	case EnPlayerState::Swing:
		m_stateMachine.AddState<PlayerSwingState>(id);
		break;
	case EnPlayerState::Num:		// State ではない（個数）ので作らない
		break;
	}
}

void BattleCharacter::InitAbilities()
{
	m_speed.Reset();
	if (HasAbility("Grapple"))
	{
		m_grapple = std::make_unique<Grapple>(m_param->lock);
		m_grapple->Init();
	}
	if (HasAbility("Gun"))
	{
		m_gun = std::make_unique<Gun>();
		m_gun->Init(FindGO<BulletManager>("bulletManager"));
	}
}
