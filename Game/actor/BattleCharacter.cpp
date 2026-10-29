#include "stdafx.h"
#include "actor/BattleCharacter.h"
#include "controller/IController.h"

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
