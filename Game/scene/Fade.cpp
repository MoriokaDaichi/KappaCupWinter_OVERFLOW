#include "stdafx.h"
#include "scene/Fade.h"

bool Fade::Start()
{
	m_sprite.Init("Assets/modelData/debug/debug_white.DDS",
		static_cast<float>(g_graphicsEngine->GetFrameBufferWidth()),
		static_cast<float>(g_graphicsEngine->GetFrameBufferHeight()));
	m_message.Init(HudText::MAX_LENGTH, MESSAGE_HEIGHT, HudText::Align::Center);
	m_message.SetColor(Vector4(0.0f, 0.9f, 1.0f, 1.0f));
	m_message.SetPosition(Vector2::Zero);
	return true;
}

void Fade::SetMessage(const char* message)
{
	m_message.SetText(message);
	m_hasMessage = true;
}

void Fade::StartFadeOut(const float seconds)
{
	m_speed = 1.0f / max(seconds, MIN_SECONDS);
}

void Fade::StartFadeIn(const float seconds)
{
	m_hasMessage = false;
	m_speed = -1.0f / max(seconds, MIN_SECONDS);
}

void Fade::Update()
{
	m_alpha += m_speed * g_gameTime->GetFrameDeltaTime();
	m_alpha = max(0.0f, min(m_alpha, 1.0f));

	m_sprite.SetMulColor(Vector4(0.0f, 0.0f, 0.0f, m_alpha));
	m_sprite.Update();
}

void Fade::Render(RenderContext& rc)
{
	if (m_alpha > 0.0f)
	{
		m_sprite.Draw(rc);
	}
	// 黒の上に重ねる（黒より後に描く予約をする）
	if (m_hasMessage && m_alpha >= 1.0f)
	{
		m_message.Draw();
	}
}
