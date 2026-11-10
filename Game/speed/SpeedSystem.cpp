#include "stdafx.h"
#include "speed/SpeedSystem.h"

void SpeedSystem::Reset()
{
	m_baseSpeed = m_param.baseMin;
	m_bonusSpeed = 0.0f;
	m_isOverdrive = false;
}

void SpeedSystem::AddBase(const float amount)
{
	if (amount < 0.0f)
	{
		m_isOverdrive = false;
	}

	m_baseSpeed += amount;
	m_baseSpeed = min(max(m_baseSpeed, m_param.baseMin), m_param.baseMax);
	UpdateOverdrive();
}

void SpeedSystem::AddBonus(const float amount)
{
	if (m_isOverdrive && amount < 0.0f)
	{
		return;
	}

	m_bonusSpeed += amount;
	m_bonusSpeed = min(max(m_bonusSpeed, 0.0f), m_param.bonusMax);
	UpdateOverdrive();
}

void SpeedSystem::RaiseBonusTo(const float bonus)
{
	m_bonusSpeed = max(m_bonusSpeed, bonus);
	m_bonusSpeed = min(m_bonusSpeed, m_param.bonusMax);
	UpdateOverdrive();
}

int SpeedSystem::GetLevel() const
{
	int level = 1;
	for (size_t i = 0; i < m_param.levelUpSpeeds.size();i++)
	{
		if (GetSpeed() < m_param.levelUpSpeeds[i])
		{
			break;
		}
		level++;
	}
	return min(level, MAX_LEVEL);
}

float SpeedSystem::GetLevelRate() const
{
	const int level = GetLevel();
	const float speed = GetSpeed();

	const float low = (level == 1) ? m_param.baseMin : m_param.levelUpSpeeds[level - 2];
	const float high = (level == MAX_LEVEL) ? m_param.maxSpeed : m_param.levelUpSpeeds[level - 1];

	const float range = high - low;
	if (range <= 0.0f)
	{
		return 0.0f;
	}

	const float levelRate = (speed - low) / range;

	return min(max(levelRate, 0.0f), 1.0f);
}

void SpeedSystem::AddBonusPerSecond(const float perSecond)
{
	if (perSecond < 0.0f && m_isBonusKept)
	{
		return;
	}

	AddBonus(perSecond * g_gameTime->GetFrameDeltaTime());
}

void SpeedSystem::SetForDebug(const float base, const float bonus)
{
	m_isOverdrive = false;
	m_baseSpeed = min(max(base, m_param.baseMin), m_param.baseMax);
	m_bonusSpeed = min(max(bonus, 0.0f), m_param.bonusMax);
	UpdateOverdrive();
}

void SpeedSystem::ClearBonus()
{
	m_bonusSpeed = 0.0f; 
	m_isOverdrive = false; 
}

void SpeedSystem::UpdateOverdrive()
{
	if (GetSpeed() >= m_param.maxSpeed)
	{
		m_isOverdrive = true;
	}
}
