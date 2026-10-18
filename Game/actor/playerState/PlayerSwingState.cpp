#include "stdafx.h"
#include "actor/playerState/PlayerSwingState.h"

#include "actor/Player.h"

void PlayerSwingState::Enter()
{
	m_owner->StartSwing();
}

void PlayerSwingState::Update()
{
	if (m_owner->IsGrappleRelease())
	{
		m_owner->ChangeState(EnPlayerState::Air);
		return;
	}

	if (m_owner->IsJumpTrigger())
	{
		m_owner->Jump();
		m_owner->ChangeState(EnPlayerState::Air);
		return;
	}

	if (m_owner->IsOnGround() && m_owner->IsFalling())
	{
		if (m_owner->IsMoveInput())
		{
			m_owner->ChangeState(EnPlayerState::Run);
		}
		else
		{
			m_owner->ChangeState(EnPlayerState::Idle);
		}
		return;
	}

	m_owner->SwingMove();
}

void PlayerSwingState::Exit()
{
	m_owner->EndSwing();
}
