#include "stdafx.h"
#include "actor/playerState/PlayerAirState.h"

#include "actor/Player.h"

void PlayerAirState::Enter()
{
	m_owner->ConsumeGroundJump();
}

void PlayerAirState::Update()
{
	//跳んだ直後はまだ接地扱いなので、下に向かっているときだけ着地とみなす
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

	if (m_owner->CanStartWallRun() && m_owner->FindWall())
	{
		m_owner->ChangeState(EnPlayerState::WallRun);
		return;
	}

	//2 段目のジャンプ。状態は Air のまま
	if (m_owner->IsJumpTrigger() && m_owner->CanJump())
	{
		m_owner->Jump();
	}

	//空中の移動と重力
	m_owner->AirMove();
	m_owner->ApplyGravity();
}
