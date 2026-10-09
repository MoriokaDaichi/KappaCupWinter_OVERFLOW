#include "stdafx.h"
#include "Game.h"
#include "actor/Player.h"

Game::~Game()
{
	DeleteGO(m_player);
}

bool Game::Start()
{
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

	m_player = NewGO<Player>(0, "player");
	m_player->SetInput(&m_input);

	return true;
}

void Game::Update()
{
	m_input.Update();
}

void Game::Render(RenderContext& rc)
{
}
