#include "stdafx.h"
#include "actor/characterState/RunState.h"

#include "actor/BattleCharacter.h"

void RunState::Enter()
{
	m_owner->ResetJumpCount();
}

void RunState::Update()
{
	// 切り替え（落ちた・跳んだ・止まった）は切り替え表が先に見る。ここに来たら走る
	m_owner->Move();
}
