#include "stdafx.h"
#include "hud/DefeatFlash.h"

#include "actor/BattleCharacter.h"

namespace
{
	// Vector4 は constexpr にできないので、クラスの定数ではなくここに置く
	const Vector3 FLASH_COLOR(1.0f, 0.95f, 0.9f);		///< 光の色（少しだけ暖かい白）
}

void DefeatFlash::Init(const BattleCharacter* player)
{
	m_player = player;
	m_flash.Init("Assets/modelData/debug/debug_white.DDS",
		static_cast<float>(g_graphicsEngine->GetFrameBufferWidth()),
		static_cast<float>(g_graphicsEngine->GetFrameBufferHeight()));
	Reset();
}

void DefeatFlash::Reset()
{
	m_flashTimer = 0.0f;
	m_lastDefeatCount = (m_player != nullptr) ? m_player->GetDefeatCount() : 0;
}

void DefeatFlash::Update()
{
	if (m_player == nullptr)
	{
		return;
	}

	const int defeatCount = m_player->GetDefeatCount();
	if (defeatCount != m_lastDefeatCount)
	{
		m_flashTimer = FLASH_TIME;
	}
	m_lastDefeatCount = defeatCount;

	if (m_flashTimer > 0.0f)
	{
		m_flashTimer = max(m_flashTimer - g_gameTime->GetFrameDeltaTime(), 0.0f);
		m_flash.SetMulColor(Vector4(FLASH_COLOR.x, FLASH_COLOR.y, FLASH_COLOR.z, m_flashTimer / FLASH_TIME * FLASH_ALPHA));
		m_flash.Update();
	}
}

void DefeatFlash::Draw(RenderContext& rc)
{
	if (m_flashTimer > 0.0f)
	{
		m_flash.Draw(rc);
	}
}
