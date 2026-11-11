#include "stdafx.h"
#include "actor/characterState/StunState.h"

#include "actor/BattleCharacter.h"

void StunState::Enter()
{
	m_timer = 0.0f;
}

void StunState::Update()
{
	m_timer += g_gameTime->GetFrameDeltaTime();

	// 空中ではじかれた勢いのまま飛び、地面に着いたら止まっていく
	if (m_owner->IsOnGround())
	{
		m_owner->Decelerate();
	}
	else
	{
		m_owner->ApplyGravity();
	}
}

bool StunState::IsConditionMet(const EnCondition when) const
{
	if (when == EnCondition::StunOver)
	{
		return m_timer >= STUN_TIME;
	}
	return false;
}
