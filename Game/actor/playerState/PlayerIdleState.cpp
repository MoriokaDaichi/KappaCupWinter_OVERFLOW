#include "stdafx.h"
#include "actor/playerState/PlayerIdleState.h"

#include "actor/Player.h"

void PlayerIdleState::Enter()
{
	m_owner->ResetJumpCount();
	m_owner->StopHorizontal();
}

void PlayerIdleState::Update()
{
	//足場から落ちたとき
	if (!m_owner->IsOnGround())
	{
		m_owner->ChangeState(EnPlayerState::Air);
		return;
	}

	//ジャンプしたとき。先に回数を増やしておき、Air に跳んで来たことを伝える
	if (m_owner->IsJumpTrigger() && m_owner->CanJump())
	{
		m_owner->Jump();
		m_owner->ChangeState(EnPlayerState::Air);
		return;
	}

	if (m_owner->IsMoveInput())
	{
		m_owner->ChangeState(EnPlayerState::Run);
	}
}
