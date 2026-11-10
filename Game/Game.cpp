#include "stdafx.h"
#include "Game.h"
#include "actor/BattleCharacter.h"
#include "camera/GameCamera.h"
#include "param/GameParam.h"
#include "weapon/BulletSystem.h"
#include "input/InputManager.h"
#include "effect/EffectManager.h"
#include "sound/GameSound.h"

Game::~Game()
{
	DeleteGO(m_player);
	m_enemySpawner.DeleteAll();
	DeleteGO(m_gameCamera);
	DeleteGO(m_bulletSystem);
	DeleteGO(m_enemyBulletSystem);
	m_stage.DeleteAll();
	nsSound::StopAllLoops();
}

bool Game::Start()
{
	// 調整用の値を読む。ほかのオブジェクトが値を使う前に読んでおく
	nsGameParam::Load();
	nsEffect::Load();
	nsSound::Load();

	m_stage.Init();

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
	m_hud.Init(m_player, &m_checkpoint, &m_giantFirewall);
	m_levelUpFlash.Init(m_player);
	m_stage.SetPlayer(m_player);
	nsSound::SetLoop(EnSound::Bgm, true);

	const char* goalText = "GOAL";
	m_goalText.Init(static_cast<int>(strlen(goalText)), GOAL_TEXT_HEIGHT, HudText::Align::Center);
	m_goalText.SetText(goalText);
	m_goalText.SetPosition(Vector2(0.0f, GOAL_FONT_Y));
	m_goalText.SetColor(Vector4(0.0f, 0.9f, 1.0f, 1.0f));

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
	m_hud.Update();
	m_levelUpFlash.Update();
	// 足音は、プレイヤーが地上か壁を走っているときだけ（歩く敵も Run になるので、キャラの中では鳴らさない）
	const EnCharacterState playerState = m_player->GetCurrentState();
	nsSound::SetLoop(EnSound::Run, playerState == EnCharacterState::Run || playerState == EnCharacterState::WallRun);
	// スイングの「シュィーン」は、速いほど大きく高くする（スイングの速さの上限で最大）
	const float swingSpeedRate = min(m_player->GetMoveSpeed().Length() / m_player->GetParam().swing.maxSpeed, 1.0f);
	const float swingVolumeRate = SWING_MIN_VOLUME_RATE + (1.0f - SWING_MIN_VOLUME_RATE) * swingSpeedRate;
	const float swingPitch = SWING_MIN_PITCH + (SWING_MAX_PITCH - SWING_MIN_PITCH) * swingSpeedRate;
	nsSound::SetLoop(EnSound::Swing, playerState == EnCharacterState::Swing, swingVolumeRate, swingPitch);
}

void Game::Render(RenderContext& rc)
{
	m_stage.Draw(rc);
	m_levelUpFlash.Draw(rc);
	m_hud.Draw();
	m_lockOnHud.Draw(rc);
	m_giantFirewall.Draw(rc);

	if (m_giantFirewall.IsBroken())
	{
		m_goalText.Draw();
	}
}
