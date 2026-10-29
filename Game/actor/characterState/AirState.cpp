#include "stdafx.h"
#include "actor/characterState/AirState.h"

#include "actor/BattleCharacter.h"

void AirState::Enter()
{
	m_owner->ConsumeGroundJump();
}

void AirState::Update()
{
	// 切り替え（着地した・壁を見つけた）は切り替え表が先に見る
	//2 段目のジャンプ。状態は Air のまま
	if (m_owner->IsJumpTrigger() && m_owner->CanJump())
	{
		m_owner->Jump();
	}

	//空中の移動と重力
	m_owner->AirMove();
	m_owner->ApplyGravity();
}
