#include "stdafx.h"
#include "hud/LevelUpFlash.h"

#include "actor/BattleCharacter.h"

namespace
{
	// Vector4 は constexpr にできないので、クラスの定数ではなくここに置く
	const Vector3 FLASH_COLOR(0.0f, 0.9f, 1.0f);		///< 光の色（シアン #00e5ff）
}

void LevelUpFlash::Init(const BattleCharacter* player)
{
	m_player = player;
	m_lastLevel = player->GetSpeedSystem().GetLevel();
	m_flash.Init("Assets/modelData/debug/debug_white.DDS",
		static_cast<float>(g_graphicsEngine->GetFrameBufferWidth()),
		static_cast<float>(g_graphicsEngine->GetFrameBufferHeight()));
}

void LevelUpFlash::Update()
{
	if (m_player == nullptr)
	{
		return;
	}

	// 下がったときも覚え直す（そうしないと、下がってからまた上がったときに光らない）
	const int level = m_player->GetSpeedSystem().GetLevel();
	if (level > m_lastLevel)
	{
		m_flashTimer = FLASH_TIME;
	}
	m_lastLevel = level;

	if (m_flashTimer > 0.0f)
	{
		m_flashTimer = max(m_flashTimer - g_gameTime->GetFrameDeltaTime(), 0.0f);
		m_flash.SetMulColor(Vector4(FLASH_COLOR.x, FLASH_COLOR.y, FLASH_COLOR.z, m_flashTimer / FLASH_TIME * FLASH_ALPHA));
		m_flash.Update();
	}
}

void LevelUpFlash::Draw(RenderContext& rc)
{
	if (m_flashTimer > 0.0f)
	{
		m_flash.Draw(rc);
	}
}
