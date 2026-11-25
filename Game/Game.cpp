#include "stdafx.h"
#include "Game.h"
#include "actor/BattleCharacter.h"
#include "camera/GameCamera.h"
#include "param/GameParam.h"
#include "weapon/BulletSystem.h"
#include "input/InputManager.h"
#include "effect/EffectManager.h"
#include "sound/GameSound.h"
#include "scene/Fade.h"
#include "scene/ResultScene.h"
#include "scene/TitleScene.h"

Game::~Game()
{
	// ポーズのまま消えても、次のシーンの時間が止まらないように戻す
	if (g_gameTime != nullptr)
	{
		g_gameTime->DisableFixedFrameDeltaTime();
	}
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

	m_enemySpawner.Init(m_player);

	m_gameCamera = NewGO<GameCamera>(1, "gameCamera");
	m_gameCamera->SetTarget(m_player);
	m_gameCamera->SetInput(&InputManager::GetInstance());

	m_debugManager.Init(m_player);
	m_lockOnHud.Init(m_player);
	m_checkpoint.Init(m_player);
	m_giantFirewall.Init(m_player);
	m_hud.Init(m_player, &m_checkpoint, &m_giantFirewall);
	m_levelUpFlash.Init(m_player);
	m_defeatFlash.Init(m_player);
	m_stage.SetPlayer(m_player);
	nsSound::SetLoop(EnSound::Bgm, true);

	const char* goalText = "GOAL";
	m_goalText.Init(static_cast<int>(strlen(goalText)), GOAL_TEXT_HEIGHT, HudText::Align::Center);
	m_goalText.SetText(goalText);
	m_goalText.SetPosition(Vector2(0.0f, GOAL_FONT_Y));
	m_goalText.SetColor(Vector4(0.0f, 0.9f, 1.0f, 1.0f));

	m_pauseMenu.Init();
	m_fade = FindGO<Fade>(Fade::NAME);
	if (m_fade != nullptr)
	{
		m_fade->StartFadeIn(FADE_TIME);
	}

	return true;
}

void Game::Update()
{
	InputManager& input = InputManager::GetInstance();
	input.Update();

	if (m_leaveTo != LEAVE_NONE)
	{
		UpdateLeave();
		return;
	}
	if (m_isPaused)
	{
		UpdatePause();
		return;
	}
	// ポーズを閉じたフレームは、閉じた A をプレイヤーに渡さないように、次のフレームで入力を戻す
	if (m_isInputRestorePending)
	{
		m_playerController.SetInput(&input);
		m_isInputRestorePending = false;
	}
	if (input.IsTrigger(EnInputAction::Pause))
	{
		SetPaused(true);
		return;
	}

	m_debugManager.Update();
	// Back ボタン（キーボードはスペース）でも最初からやり直せる。デバッグ画面のない Release でも使える（試遊会用）
	const bool isRestartPressed = InputManager::GetInstance().IsTrigger(EnInputAction::Restart);
	if (m_debugManager.ConsumeResetRequest() || isRestartPressed)
	{
		Reset();
	}
	m_stage.Update();
	m_enemySpawner.Update();
	m_checkpoint.Update();
	m_giantFirewall.Update();
	m_hud.Update();
	m_levelUpFlash.Update();
	m_defeatFlash.Update();
	// 足音は、プレイヤーが地上か壁を走っているときだけ（歩く敵も Run になるので、キャラの中では鳴らさない）
	const EnCharacterState playerState = m_player->GetCurrentState();
	nsSound::SetLoop(EnSound::Run, playerState == EnCharacterState::Run || playerState == EnCharacterState::WallRun);
	// スイングの「シュィーン」は、速いほど大きく高くする（スイングの速さの上限で最大）
	const float swingSpeedRate = min(m_player->GetMoveSpeed().Length() / m_player->GetParam().swing.maxSpeed, 1.0f);
	const float swingVolumeRate = SWING_MIN_VOLUME_RATE + (1.0f - SWING_MIN_VOLUME_RATE) * swingSpeedRate;
	const float swingPitch = SWING_MIN_PITCH + (SWING_MAX_PITCH - SWING_MIN_PITCH) * swingSpeedRate;
	nsSound::SetLoop(EnSound::Swing, playerState == EnCharacterState::Swing, swingVolumeRate, swingPitch);

	// 突破したら、GOAL の文字を少し見せてからリザルトへ
	if (m_giantFirewall.IsBroken())
	{
		m_goalTimer += g_gameTime->GetFrameDeltaTime();
		if (m_goalTimer >= GOAL_TO_RESULT_TIME)
		{
			StartLeave(LEAVE_TO_RESULT);
		}
	}
}

void Game::SetPaused(const bool isPaused)
{
	m_isPaused = isPaused;
	if (isPaused)
	{
		// ほかのオブジェクトは g_gameTime の 1 フレームの時間で動くので、0 にすると全部止まる（描くのは続く）
		g_gameTime->EnableFixedFrameDeltaTime(0.0f);
		// 時間が 0 でも、ジャンプや射撃のボタンは効いてしまうので、プレイヤーへの入力を止める
		m_playerController.SetInput(nullptr);
		StopLoopSounds();
		m_pauseMenu.Open();
		return;
	}

	g_gameTime->DisableFixedFrameDeltaTime();
	m_isInputRestorePending = true;
}

void Game::UpdatePause()
{
	switch (m_pauseMenu.Update(InputManager::GetInstance()))
	{
	case PauseMenu::Choice::Resume:
		SetPaused(false);
		break;
	case PauseMenu::Choice::Checkpoint:
		SetPaused(false);
		m_checkpoint.Respawn();
		break;
	case PauseMenu::Choice::Title:
		SetPaused(false);
		StartLeave(LEAVE_TO_TITLE);
		break;
	case PauseMenu::Choice::None:
		break;
	}
}

void Game::StartLeave(const int next)
{
	m_leaveTo = next;
	m_playerController.SetInput(nullptr);
	m_isInputRestorePending = false;
	StopLoopSounds();
	if (m_fade != nullptr)
	{
		m_fade->StartFadeOut(FADE_TIME);
	}
}

void Game::UpdateLeave()
{
	if (m_fade != nullptr && !m_fade->IsBlack())
	{
		return;
	}

	if (m_leaveTo == LEAVE_TO_RESULT)
	{
		ResultScene* result = NewGO<ResultScene>(0, "result");
		result->SetResult(m_hud.GetTime(), m_hud.GetAverageLevel(), m_player->GetDefeatCount());
	}
	else
	{
		NewGO<TitleScene>(0, "title");
	}
	DeleteGO(this);
	// 消えるのはフレームの終わりなので、次のシーンを 2 回作らないように、もう抜けたことにする
	m_leaveTo = LEAVE_NONE;
	m_fade = nullptr;
}

void Game::StopLoopSounds()
{
	nsSound::SetLoop(EnSound::Run, false);
	nsSound::SetLoop(EnSound::Swing, false);
}

void Game::Reset()
{
	const std::array<float, 3>& playerStart = nsGameParam::Get().stage.playerStart;
	m_player->Respawn(Vector3(playerStart[0], playerStart[1], playerStart[2]));
	m_checkpoint.Reset();
	m_giantFirewall.Reset();
	m_hud.Reset();
	m_defeatFlash.Reset();
	m_goalTimer = 0.0f;
	m_enemySpawner.Reset();
}

void Game::Render(RenderContext& rc)
{
	m_stage.Draw(rc);
	m_levelUpFlash.Draw(rc);
	m_defeatFlash.Draw(rc);
	m_hud.Draw(rc);
	m_lockOnHud.Draw(rc);
	m_giantFirewall.Draw(rc);

	if (m_giantFirewall.IsBroken())
	{
		m_goalText.Draw();
	}

	// メニューは HUD の上に重ねるので、最後に描く
	if (m_isPaused)
	{
		m_pauseMenu.Draw(rc);
	}
}
