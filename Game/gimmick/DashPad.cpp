#include "stdafx.h"
#include "gimmick/DashPad.h"

#include "actor/BattleCharacter.h"
#include "sound/GameSound.h"

void DashPad::InitView()
{
	m_view.Init("Assets/modelData/gimmick/gimmick_dashpad.tkm", GimmickView::Kind::DashPad, 1);

	if (m_wallSide == FLOOR)
	{
		// 判定の箱の底に、箱と同じ幅と奥行で置く（厚さはモデルのまま）
		const Vector3 bottom = Vector3(m_position.x, m_position.y - m_size.y * 0.5f, m_position.z);
		m_view.Add(bottom, Vector3(m_size.x / MODEL_SIZE, 1.0f, m_size.z / MODEL_SIZE));
		return;
	}

	// 壁の面（箱の ±x の面）に、箱と同じ高さと奥行で立てる。板の幅（モデルの x）が高さの向きになる
	// +90 度回すと板の表（モデルの +y）が -x を向くので、+x の壁に貼るときは +90 度、-x の壁は -90 度
	const float wallX = m_position.x + m_size.x * 0.5f * static_cast<float>(m_wallSide);
	const float roll = Math::PI * 0.5f * static_cast<float>(m_wallSide);
	m_view.Add(Vector3(wallX, m_position.y, m_position.z), Vector3(m_size.y / MODEL_SIZE, 1.0f, m_size.z / MODEL_SIZE), roll);
}

void DashPad::OnEnter(BattleCharacter& player)
{
	nsSound::Play(EnSound::DashPad);
	player.GetSpeedSystem().AddBonus(BOOST);
	// 上乗せが上限だと AddBonus では増えないので、上限とは別のサージでも加速させる（いつ乗っても速くなる）
	player.GetSpeedSystem().StartSurge();
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
