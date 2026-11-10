#include "stdafx.h"
#include "gimmick/DashPad.h"

#include "actor/BattleCharacter.h"

void DashPad::InitView()
{
	m_view.Init("Assets/modelData/gimmick/gimmick_dashpad.tkm", GimmickView::Kind::DashPad, 1);

	// 判定の箱の底に、箱と同じ幅と奥行で置く（厚さはモデルのまま）
	const Vector3 bottom = Vector3(m_position.x, m_position.y - m_size.y * 0.5f, m_position.z);
	m_view.Add(bottom, Vector3(m_size.x / MODEL_SIZE, 1.0f, m_size.z / MODEL_SIZE));
}

void DashPad::OnEnter(BattleCharacter& player)
{
	player.GetSpeedSystem().AddBonus(BOOST);
	player.AddMoveSpeed(Vector3(0.0f, 0.0f, PUSH_SPEED));
}

void DashPad::OnStay(BattleCharacter& player)
{
	player.GetSpeedSystem().KeepBonus();
}

void DashPad::Draw(RenderContext& rc)
{
	m_view.Draw(1.0f);
}
