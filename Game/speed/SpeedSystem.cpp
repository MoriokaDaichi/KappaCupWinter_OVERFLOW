#include "stdafx.h"
#include "speed/SpeedSystem.h"

void SpeedSystem::Reset()
{
	m_baseSpeed = m_param.baseMin;
	m_bonusSpeed = 0.0f;
}

void SpeedSystem::AddBase(float amount)
{
	m_baseSpeed += amount;
	m_baseSpeed = min(max(m_baseSpeed, m_param.baseMin), m_param.baseMax);
}

void SpeedSystem::AddBonus(float amount)
{
	m_bonusSpeed += amount;
	m_bonusSpeed = min(max(m_bonusSpeed, 0.0f), m_param.bonusMax);
}

void SpeedSystem::RaiseBonusTo(float bonus)
{
	m_bonusSpeed = max(m_bonusSpeed, bonus);
	m_bonusSpeed = min(m_bonusSpeed, m_param.bonusMax);
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

void SpeedSystem::AddBonusPerSecond(float perSecond)
{
	AddBonus(perSecond * g_gameTime->GetFrameDeltaTime());
}
