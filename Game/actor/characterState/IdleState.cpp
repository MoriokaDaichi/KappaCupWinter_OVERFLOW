#include "stdafx.h"
#include "actor/characterState/IdleState.h"

#include "actor/BattleCharacter.h"

void IdleState::Enter()
{
	m_owner->ResetJumpCount();
}

void IdleState::Update()
{
	// 切り替え（落ちた・跳んだ・歩き出した）は切り替え表が先に見る。ここに来たらスティックを離しているので、少しずつ止まる
	m_owner->Decelerate();
}
