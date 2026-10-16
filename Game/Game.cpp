#include "stdafx.h"
#include "Game.h"
#include "actor/Player.h"
#include "camera/GameCamera.h"
#include "gimmick/Anchor.h"

Game::~Game()
{
	DeleteGO(m_player);
	DeleteGO(m_gameCamera);
	for (int i = 0; i < ANCHOR_NUM; i++)
	{
		DeleteGO(m_anchor[i]);
	}
}

bool Game::Start()
{
	// 仮の床（ステージができたら置き換える）
	m_floorCollider.Create(Vector3(20000.0f, 10.0f, 20000.0f));

	RigidBodyInitData floorData;
	floorData.pos = Vector3(0.0f, -5.0f, 0.0f);		//厚さが10の箱なので、上の面がy=0になるように半分下げる
	floorData.collider = &m_floorCollider;			//どの形を使うか（ポインタで渡す）
	//massは書かなければ0→動かない物体になる
	m_floorBody.Init(floorData);					//ここで物理世界に登録

	m_floorBody.GetBody()->setUserIndex(enCollisionAttr_Ground);	//種類は地面

	m_floorRender.Init("Assets/modelData/debug/debug_floor.tkm");
	m_floorRender.SetPosition(floorData.pos);	// 当たり判定と同じ場所
	m_floorRender.Update();

	//仮の壁（ステージができたら置き換える）
	const Vector3 wallSize(200.0f, 800.0f, 10000.0f);	// 幅・高さ・長さ
	const Vector3 wallPos[WALL_NUM] =			// 床の上に立つように、高さの半分だけ上げる
	{
		Vector3(300.0f, wallSize.y * 0.5f, 0.0f),
		Vector3(-300.0f, wallSize.y * 0.5f, 0.0f),
	};

	for (int i = 0; i < WALL_NUM; i++)
	{
		m_wallCollider[i].Create(wallSize);

		RigidBodyInitData wallData;
		wallData.pos = wallPos[i];
		wallData.collider = &m_wallCollider[i];
		m_wallBody[i].Init(wallData);

		m_wallRender[i].Init("Assets/modelData/debug/debug_wall.tkm");
		m_wallRender[i].SetPosition(wallPos[i]);
		m_wallRender[i].SetScale(wallSize / 100.0f);
		m_wallRender[i].Update();
	}
	
	const Vector3 anchorPos[ANCHOR_NUM] =
	{
		Vector3(0.0f, 800.0f, 1500.0f),	// 正面の上
		Vector3(600.0f, 700.0f, 1000.0f),	// 右の壁の上
		Vector3(-600.0f, 700.0f, 1000.0f),	// 左の壁の上
		Vector3(0.0f, 600.0f, 3500.0f),	// 正面の遠く
	};

	for (int i = 0; i < ANCHOR_NUM; i++)
	{
		m_anchor[i] = NewGO<Anchor>(0, "anchor");
		m_anchor[i]->SetPosition(anchorPos[i]);
	}

	// 当たり判定の形を線で表示する（デバッグ用）
	PhysicsWorld::GetInstance()->EnableDrawDebugWireFrame();

	m_player = NewGO<Player>(0, "player");
	m_player->SetInput(&m_input);

	m_gameCamera = NewGO<GameCamera>(1, "gameCamera");
	m_gameCamera->SetTarget(m_player);
	m_gameCamera->SetInput(&m_input);

	return true;
}

void Game::Update()
{
	m_input.Update();
}

void Game::Render(RenderContext& rc)
{
	m_floorRender.Draw(rc);
	for (int i = 0; i < WALL_NUM; i++)
	{
		m_wallRender[i].Draw(rc);
	}
}
