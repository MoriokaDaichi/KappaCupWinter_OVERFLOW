#include "stdafx.h"
#include "Game.h"
#include "actor/Player.h"
#include "actor/Enemy.h"
#include "camera/GameCamera.h"
#include "gimmick/Anchor.h"
#include "param/GameParam.h"
#include "weapon/BulletManager.h"
#include "input/InputManager.h"

Game::~Game()
{
	DeleteGO(m_player);
	DeleteGO(m_enemy);
	DeleteGO(m_gameCamera);
	DeleteGO(m_bulletManager);
	for (int i = 0; i < ANCHOR_NUM; i++)
	{
		DeleteGO(m_anchor[i]);
	}
}

bool Game::Start()
{
	// 調整用の値を読む。ほかのオブジェクトが値を使う前に読んでおく
	nsGameParam::Load();

	// 仮の床（ステージができたら置き換える）
	m_floorCollider.Create(Vector3(FLOOR_WIDTH, FLOOR_THICKNESS, FLOOR_WIDTH));

	RigidBodyInitData floorData;
	floorData.pos = Vector3(0.0f, -FLOOR_THICKNESS * 0.5f, 0.0f);		//上の面がy=0になるように、厚さの半分だけ下げる
	floorData.collider = &m_floorCollider;			//どの形を使うか（ポインタで渡す）
	//massは書かなければ0→動かない物体になる
	m_floorBody.Init(floorData);					//ここで物理世界に登録

	m_floorBody.GetBody()->setUserIndex(enCollisionAttr_Ground);	//種類は地面

	m_floorRender.Init("Assets/modelData/debug/debug_floor.tkm");
	m_floorRender.SetPosition(floorData.pos);	// 当たり判定と同じ場所
	m_floorRender.Update();

	//仮の壁（ステージができたら置き換える）
	//壁走りの通路は後ろ側（z = -5500 〜 -500）に置き、前側はグラップルのテスト用に空けておく
	const Vector3 wallSize(200.0f, 800.0f, 5000.0f);	// 幅・高さ・長さ
	const Vector3 wallPos[WALL_NUM] =			// 床の上に立つように、高さの半分だけ上げる
	{
		Vector3(300.0f, wallSize.y * 0.5f, -3000.0f),
		Vector3(-300.0f, wallSize.y * 0.5f, -3000.0f),
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
		m_wallRender[i].SetScale(wallSize / DEBUG_MODEL_SIZE);
		m_wallRender[i].Update();
	}
	
	const Vector3 anchorPos[ANCHOR_NUM] =
	{
		// 前側の広い場所：左右に振りながら奥へ続けてスイングできるように並べる
		Vector3(0.0f, 900.0f, 1500.0f),
		Vector3(-800.0f, 1000.0f, 3000.0f),
		Vector3(800.0f, 1000.0f, 4500.0f),
		Vector3(-800.0f, 1100.0f, 6000.0f),
		Vector3(800.0f, 1100.0f, 7500.0f),
		Vector3(0.0f, 1200.0f, 9000.0f),
		// 後ろ側の通路：壁走りからつなげる
		Vector3(600.0f, 1000.0f, -3000.0f),	// 右の壁の上
		Vector3(-600.0f, 1000.0f, -3000.0f),	// 左の壁の上
	};

	for (int i = 0; i < ANCHOR_NUM; i++)
	{
		m_anchor[i] = NewGO<Anchor>(0, "anchor");
		m_anchor[i]->SetPosition(anchorPos[i]);
	}

	m_crosshairRender.Init("Assets/sprite/debug/debug_crosshair.DDS", CROSSHAIR_SIZE, CROSSHAIR_SIZE);

	// 当たり判定の形を線で表示する（デバッグ用）
	PhysicsWorld::GetInstance()->EnableDrawDebugWireFrame();

	m_bulletManager = NewGO<BulletManager>(0, "bulletManager");

	m_player = NewGO<Player>(0, "player");
	m_playerController.SetInput(&InputManager::GetInstance());
	m_player->SetController(&m_playerController);

	// 仮: ステップ 5 の確認用。アンカーから離れた横の方に置き、プレイヤーの方へ歩かせる
	m_enemy = NewGO<Enemy>(0, "enemy");
	m_enemy->SetStartPosition(Vector3(2500.0f, START_HEIGHT, 0.0f));
	m_enemyController.SetOwner(m_enemy);
	m_enemyController.SetTarget(m_player);
	m_enemy->SetController(&m_enemyController);

	m_gameCamera = NewGO<GameCamera>(1, "gameCamera");
	m_gameCamera->SetTarget(m_player);
	m_gameCamera->SetInput(&InputManager::GetInstance());

	m_debugManager.Init(m_player);

	return true;
}

void Game::Update()
{
	InputManager::GetInstance().Update();

	m_debugManager.Update();
}

void Game::Render(RenderContext& rc)
{
	m_floorRender.Draw(rc);

	for (int i = 0; i < WALL_NUM; i++)
	{
		m_wallRender[i].Draw(rc);
	}

	m_crosshairRender.Draw(rc);
}
