#include "stdafx.h"
#include "actor/playerState/PlayerWallRunState.h"

#include "actor/Player.h"

void PlayerWallRunState::Enter()
{
	m_owner->StartWallRun();
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

	m_owner->WallRunMove();
}

void PlayerWallRunState::Exit()
{
	m_owner->EndWallRun();
}
