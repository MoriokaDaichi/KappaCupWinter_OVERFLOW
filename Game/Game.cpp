#include "stdafx.h"
#include "Game.h"


bool Game::Start()
{
	m_modelRender.Init("Assets/modelData/unityChan.tkm");

	m_debugPanel.Register("Game", "Input", [this]() {
		ImGui::Text("Shoot  press:%d  trig:%d  rel:%d  hold:%.2f",
			m_input.IsPress(EnInputAction::Shoot),
			m_input.IsTrigger(EnInputAction::Shoot),
			m_input.IsRelease(EnInputAction::Shoot),
			m_input.GetHoldTime(EnInputAction::Shoot));

		const Vector2 move = m_input.GetMove();
		const Vector2 camera = m_input.GetCamera();
		ImGui::Text("Move   x:%.2f  y:%.2f", move.x, move.y);
		ImGui::Text("Camera x:%.2f  y:%.2f", camera.x, camera.y);
		}, true);

	return true;
}

void Game::Update()
{
	// g_renderingEngine->DisableRaytracing();
	m_modelRender.Update();
	m_input.Update();
}

void Game::Render(RenderContext& rc)
{
	m_modelRender.Draw(rc);
}
