#include "stdafx.h"
#include "gimmick/ElectricFence.h"

#include "actor/BattleCharacter.h"

void ElectricFence::Draw(RenderContext& rc)
{
	m_posts.Draw(1.0f);
	m_wires.Draw(1.0f);
}

void ElectricFence::InitView()
{
	const float bottom = m_position.y - m_size.y * 0.5f;
	const float left = m_position.x - m_size.x * 0.5f;

	// 両端と、その間に等間隔で柱を立てる
	const int spanCount = (std::max)(1, static_cast<int>(std::ceil(m_size.x / POST_INTERVAL)));
	m_posts.Init("Assets/modelData/gimmick/gimmick_fence_post.tkm", GimmickView::Kind::Fence, spanCount + 1);
	for (int i = 0; i <= spanCount; i++)
	{
		const float x = left + m_size.x * static_cast<float>(i) / static_cast<float>(spanCount);
		m_posts.Add(Vector3(x, bottom, m_position.z), Vector3::One);
	}

	m_wires.Init("Assets/modelData/gimmick/gimmick_fence_wire.tkm", GimmickView::Kind::Fence, 1);
	m_wires.Add(Vector3(m_position.x, bottom, m_position.z), Vector3(m_size.x / WIRE_LENGTH, 1.0f, 1.0f));
}

void ElectricFence::OnEnter(BattleCharacter& player)
{
	player.TakeDamage(DAMAGE);
	player.GetSpeedSystem().ClearBonus();
}
