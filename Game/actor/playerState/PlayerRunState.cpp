#include "stdafx.h"
#include "actor/playerState/PlayerRunState.h"

#include "actor/BattleCharacter.h"

void PlayerRunState::Enter()
{
	m_owner->ResetJumpCount();
}

void PlayerRunState::Update()
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

	if (!m_owner->IsMoveInput())
	{
		m_owner->ChangeState(EnPlayerState::Idle);
		return;
	}

	m_owner->Move();
}
