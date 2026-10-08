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

	// 仮の床（ステージができたら置き換える）
	m_floorCollider.Create(Vector3(2000.0f, 10.0f, 2000.0f));

	RigidBodyInitData floorData;
	floorData.pos = Vector3(0.0f, -5.0f, 0.0f);		//厚さが10の箱なので、上の面がy=0になるように半分下げる
	floorData.collider = &m_floorCollider;			//どの形を使うか（ポインタで渡す）
	//massは書かなければ0→動かない物体になる
	m_floorBody.Init(floorData);					//ここで物理世界に登録

	m_floorBody.GetBody()->setUserIndex(enCollisionAttr_Ground);	//種類は地面

	// 当たり判定の形を線で表示する（デバッグ用）
	PhysicsWorld::GetInstance()->EnableDrawDebugWireFrame();

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
