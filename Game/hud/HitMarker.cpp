#include "stdafx.h"
#include "hud/HitMarker.h"

#include "actor/BattleCharacter.h"

namespace
{
	// Vector4 は constexpr にできないので、クラスの定数ではなくここに置く
	const Vector4 HIT_COLOR(1.0f, 1.0f, 1.0f, 1.0f);		///< 当たったときの白
	const Vector4 DEFEAT_COLOR(1.0f, 0.2f, 0.35f, 1.0f);	///< 倒したときの赤（敵の輪郭線と同じ系統）
}

void HitMarker::Init(const BattleCharacter* player)
{
	m_player = player;
	m_hitMark.Init("Assets/shader/hudHitMarker.fx", HIT_SIZE, HIT_SIZE);
	m_defeatMark.Init("Assets/shader/hudHitMarker.fx", DEFEAT_SIZE, DEFEAT_SIZE);
	Reset();
}

void HitMarker::Reset()
{
	m_hitTimer = 0.0f;
	m_defeatTimer = 0.0f;
	m_lastHitCount = (m_player != nullptr) ? m_player->GetHitCount() : 0;
	m_lastDefeatCount = (m_player != nullptr) ? m_player->GetDefeatCount() : 0;
}

void HitMarker::Update()
{
	if (m_player == nullptr)
	{
		return;
	}

	// 1 フレームに何発当たっても 1 回出せばよいので、前のフレームと違うかだけを見る（何発増えたかは数えない）
	const int hitCount = m_player->GetHitCount();
	if (hitCount != m_lastHitCount)
	{
		m_hitTimer = HIT_TIME;
	}
	m_lastHitCount = hitCount;

	const int defeatCount = m_player->GetDefeatCount();
	if (defeatCount != m_lastDefeatCount)
	{
		m_defeatTimer = DEFEAT_TIME;
	}
	m_lastDefeatCount = defeatCount;

	const float dt = g_gameTime->GetFrameDeltaTime();
	m_hitTimer = max(m_hitTimer - dt, 0.0f);
	m_defeatTimer = max(m_defeatTimer - dt, 0.0f);
}

void HitMarker::Draw()
{
	// 倒したときは、当たったときのマークを重ねない（赤い大きいほうだけを見せる）
	if (m_defeatTimer > 0.0f)
	{
		m_defeatMark.Update(Vector2::Zero, DEFEAT_COLOR, m_defeatTimer / DEFEAT_TIME);
		m_defeatMark.Draw();
		return;
	}
	if (m_hitTimer > 0.0f)
	{
		m_hitMark.Update(Vector2::Zero, HIT_COLOR, m_hitTimer / HIT_TIME);
		m_hitMark.Draw();
	}
}
