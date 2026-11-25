#include "stdafx.h"
#include "stage/Checkpoint.h"

#include "actor/BattleCharacter.h"
#include "param/GameParam.h"

void Checkpoint::Init(BattleCharacter* player)
{
	m_player = player;
	m_index = 0;
}

void Checkpoint::Update()
{
	const std::vector<StageParam::Section>& sections = nsGameParam::Get().stage.sections;
	const Vector3& position = m_player->GetPosition();

	const int next = m_index + 1;
	if (next < static_cast<int>(sections.size()) && position.z >= sections[next].checkpoint[2])
	{
		m_index = next;
	}

	// 落ちた後は、SIGNAL LOST を見せ終わってから戻す
	if (m_signalLostTimer > 0.0f)
	{
		m_signalLostTimer -= g_gameTime->GetFrameDeltaTime();
		if (m_signalLostTimer <= 0.0f)
		{
			Respawn();
		}
		return;
	}

	if (position.y < FALL_HEIGHT)
	{
		m_signalLostTimer = SIGNAL_LOST_TIME;
	}
}

void Checkpoint::Respawn()
{
	m_signalLostTimer = 0.0f;

	const std::array<float, 3>& checkpoint = nsGameParam::Get().stage.sections[m_index].checkpoint;
	const Vector3 position(checkpoint[0], checkpoint[1], checkpoint[2]);

	m_player->Warp(position);
	m_player->GetSpeedSystem().ClearBonus();
}
