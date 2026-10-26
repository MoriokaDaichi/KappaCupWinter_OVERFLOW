#include "stdafx.h"
#include "actor/playerState/PlayerAirState.h"

#include "actor/Player.h"
#include "actor/playerState/PlayerWallRunState.h"

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

	Vector3 wallNormal;
	// 仮: State から別の State を呼んでいる。ステップ 6 で切り替え表の条件に移す
	if (m_owner->CanStartWallRun() && PlayerWallRunState::FindWall(*m_owner, wallNormal))
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
