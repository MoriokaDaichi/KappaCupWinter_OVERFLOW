#include "stdafx.h"
#include "Game.h"
#include "actor/BattleCharacter.h"
#include "camera/GameCamera.h"
#include "param/GameParam.h"
#include "weapon/BulletSystem.h"
#include "input/InputManager.h"
#include "effect/EffectManager.h"

Game::~Game()
{
	DeleteGO(m_player);
	m_enemySpawner.DeleteAll();
	DeleteGO(m_gameCamera);
	DeleteGO(m_bulletSystem);
	DeleteGO(m_enemyBulletSystem);
	m_stage.DeleteAll();
}

bool Game::Start()
{
	// 調整用の値を読む。ほかのオブジェクトが値を使う前に読んでおく
	nsGameParam::Load();
	nsEffect::Load();

	m_stage.Init();

	m_crosshairRender.Init("Assets/sprite/debug/debug_crosshair.DDS", CROSSHAIR_SIZE, CROSSHAIR_SIZE);

	m_bulletSystem = NewGO<BulletSystem>(0, "bulletSystem");
	m_bulletSystem->Init("enemy");

	m_enemyBulletSystem = NewGO<BulletSystem>(0, "enemyBulletSystem");
	m_enemyBulletSystem->Init("player");

	// プレイヤーも敵も同じ BattleCharacter。違うのは、渡す設定（JSON）と Controller だけ
	m_player = NewGO<BattleCharacter>(0, "player");
	const std::array<float, 3>& playerStart = nsGameParam::Get().stage.playerStart;
	m_player->SetStartPosition(Vector3(playerStart[0], playerStart[1], playerStart[2]));
	m_playerController.SetInput(&InputManager::GetInstance());
	m_player->SetController(&m_playerController);
	m_player->SetParam(nsGameParam::Get().player);

	m_enemySpawner.Init(m_player, m_enemyBulletSystem);

	m_gameCamera = NewGO<GameCamera>(1, "gameCamera");
	m_gameCamera->SetTarget(m_player);
	m_gameCamera->SetInput(&InputManager::GetInstance());

	m_debugManager.Init(m_player);
	m_lockOnHud.Init(m_player);
	m_checkpoint.Init(m_player);
	m_giantFirewall.Init(m_player);

	return true;
}

void Game::Update()
{
	InputManager::GetInstance().Update();

	m_debugManager.Update();
	m_stage.Update();
	m_enemySpawner.Update();
	m_checkpoint.Update();
	m_giantFirewall.Update();
}

void Game::Render(RenderContext& rc)
{
	m_stage.Draw();
	m_crosshairRender.Draw(rc);
	m_lockOnHud.Draw(rc);
	m_giantFirewall.Draw(rc);
}
