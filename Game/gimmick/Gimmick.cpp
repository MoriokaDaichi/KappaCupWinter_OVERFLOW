#include "stdafx.h"
#include "gimmick/Gimmick.h"

#include "actor/BattleCharacter.h"

void Gimmick::Init(const StageParam::Block& block)
{
	m_position = Vector3(block.position[0], block.position[1], block.position[2]);
	m_size = Vector3(block.size[0], block.size[1], block.size[2]);
	InitView();
}

void Gimmick::Update(BattleCharacter& player)
{
	UpdateGimmick();

	const Vector3& foot = player.GetPosition();
	const Vector3 toFoot = foot - m_position;
	const bool isInside =
		IsActive() &&
		fabsf(toFoot.x) <= m_size.x * 0.5f &&
		fabsf(toFoot.y) <= m_size.y * 0.5f &&
		fabsf(toFoot.z) <= m_size.z * 0.5f;

	if (isInside && !m_isPlayerInside)
	{
		OnEnter(player);
	}

	if (isInside)
	{
		OnStay(player);
	}

	m_isPlayerInside = isInside;
}
