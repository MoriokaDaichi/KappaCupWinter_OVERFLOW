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
#include "debug/LoadTimer.h"

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
	// 最初の 1 回だけ：時間を止めて、やることを並べる
	if (m_loadTasks.empty())
	{
		// 読み込みの間は時間を 0 にする（先に作り終わったプレイヤーや敵が、遊び始める前に動かないように。ポーズと同じ仕組み）
		g_gameTime->EnableFixedFrameDeltaTime(0.0f);
		BuildLoadTasks();
		m_lastLoadTime = std::chrono::steady_clock::now();
	}

	const auto frameStart = std::chrono::steady_clock::now();

	// 1 フレームの中では、前のフレームで NewGO したもの（プレイヤーや敵）の Start が、この Start より先に動く
	// 前の Start を抜けてから長くたっている＝このフレームはそれだけでもう重いので、作業を足さずに次のフレームへ回す
	const std::chrono::duration<float> sinceLastLoad = frameStart - m_lastLoadTime;
	if (sinceLastLoad.count() >= LOAD_BUSY_SEC && !m_isLoadRested)
	{
		m_isLoadRested = true;
		m_lastLoadTime = std::chrono::steady_clock::now();
		return false;
	}
	m_isLoadRested = false;

	// 1 フレームに LOAD_BUDGET_SEC まで、作業を前から順に進める
	while (m_loadTaskIndex < static_cast<int>(m_loadTasks.size()))
	{
		// 今の作業をやる。false（まだ続きがある）なら、続きは次のフレーム
		if (!m_loadTasks[m_loadTaskIndex]())
		{
			m_lastLoadTime = std::chrono::steady_clock::now();
			return false;
		}
		m_loadTaskIndex++;

		// 予算を使い切ったら、次のフレームへ
		const std::chrono::duration<float> elapsed = std::chrono::steady_clock::now() - frameStart;
		if (elapsed.count() >= LOAD_BUDGET_SEC)
		{
			m_lastLoadTime = std::chrono::steady_clock::now();
			return false;
		}
	}
	// 全部終わった（ここで初めて IsStart() が true になり、IsReady() も true になる）
	return true;
}

void Game::Update()
{
	// 遊び始めるまで（タイトルがトンネルを見せている間）は、何もしない
	if (!m_isBegun)
	{
		return;
	}

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

void Game::Begin()
{
	m_isBegun = true;
	g_gameTime->DisableFixedFrameDeltaTime();
	if (m_fade != nullptr)
	{
		m_fade->StartFadeIn(FADE_TIME);
	}
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

void Game::BuildLoadTasks()
{
	// 調整用の値を読む。ほかのオブジェクトが値を使う前に読んでおく
	m_loadTasks.push_back([this]()
		{
			LoadTimer timer("Param::Load");
			nsGameParam::Load();
			nsEffect::Load();
			nsSound::Load();
			return true;
		});

	// 弾の係（プレイヤー用・敵用）。キャラが Start で FindGO して使うので、キャラより先に作る
	// 弾の係の Start は重い（2 つで約 0.3 秒）ので、ステージを作っている間に済ませて、プレイヤーの Start と同じフレームに重ねない
	m_loadTasks.push_back([this]()
		{
			LoadTimer timer("PlayerBullet::Init");
			m_bulletSystem = NewGO<BulletSystem>(0, "bulletSystem");
			m_bulletSystem->Init("enemy");
			return true;
		});
	m_loadTasks.push_back([this]()
		{
			LoadTimer timer("EnemyBullet::Init");
			m_enemyBulletSystem = NewGO<BulletSystem>(0, "enemyBulletSystem");
			m_enemyBulletSystem->Init("player");
			return true;
		});

	// 箱と背景の見た目
	m_loadTasks.push_back([this]()
		{
			LoadTimer timer("Stage::InitRenderer");
			m_stage.InitRenderer();
			return true;
		});
	// 舞うデータの粒
	m_loadTasks.push_back([this]()
		{
			LoadTimer timer("Stage::InitParticle");
			m_stage.InitParticle();
			return true;
		});
	// 小物の見た目（種類ごとに 1 つの作業。i はループの間に変わるので、値で取り込む）
	for (int i = 0; i < StageParam::PROP_TYPE_COUNT; i++)
	{
		m_loadTasks.push_back([this, i]()
			{
				LoadTimer timer("Stage::InitProp");
				m_stage.InitProp(i);
				return true;
			});
	}
	// 箱・アンカー・ギミックを置き始める（前の分を片づける）
	m_loadTasks.push_back([this]()
		{
			m_stage.BeginBuild();
			return true;
		});
	// 区間を 1 つずつ置く（全部置いたら true。それまでは続きを次のフレームで）
	m_loadTasks.push_back([this]()
		{
			LoadTimer timer("Stage::BuildNextSection");
			return m_stage.BuildNextSection();
		});

	// プレイヤーも敵も同じ BattleCharacter。違うのは、渡す設定（JSON）と Controller だけ
	m_loadTasks.push_back([this]()
		{
			LoadTimer timer("Player::Init");
			m_player = NewGO<BattleCharacter>(0, "player");
			const std::array<float, 3>& playerStart = nsGameParam::Get().stage.playerStart;
			m_player->SetStartPosition(Vector3(playerStart[0], playerStart[1], playerStart[2]));
			m_playerController.SetInput(&InputManager::GetInstance());
			m_player->SetController(&m_playerController);
			m_player->SetParam(nsGameParam::Get().player);
			return true;
		});

	// ここから下のカメラ・HUD・メニューは、どれもプレイヤーを使うので、プレイヤーの後に作る
	// 重いもの（ロックオン・巨大ファイアウォール・HUD・ポーズメニュー）は 1 つで 1 作業にする（軽いものは予算の中で同じフレームにまとまる）

	// カメラとデバッグ画面
	m_loadTasks.push_back([this]()
		{
			LoadTimer timer("Hud::Camera");
			m_gameCamera = NewGO<GameCamera>(1, "gameCamera");
			m_gameCamera->SetTarget(m_player);
			m_gameCamera->SetInput(&InputManager::GetInstance());
			m_debugManager.Init(m_player);
			return true;
		});

	// マルチロックオンのドーナツ
	m_loadTasks.push_back([this]()
		{
			LoadTimer timer("Hud::LockOnHud");
			m_lockOnHud.Init(m_player);
			return true;
		});

	// チェックポイントと巨大ファイアウォール（HUD が両方を使うので、HUD より先）
	m_loadTasks.push_back([this]()
		{
			LoadTimer timer("Hud::GiantFirewall");
			m_checkpoint.Init(m_player);
			m_giantFirewall.Init(m_player);
			return true;
		});

	// タイム・チェックポイント・照準・スピードメーター・グラップルの角カッコ
	m_loadTasks.push_back([this]()
		{
			LoadTimer timer("Hud::Hud");
			m_hud.Init(m_player, &m_checkpoint, &m_giantFirewall);
			return true;
		});

	// 画面の光・ギミックが見るプレイヤー・BGM・「GOAL」の文字
	m_loadTasks.push_back([this]()
		{
			LoadTimer timer("Hud::Flash");
			m_levelUpFlash.Init(m_player);
			m_defeatFlash.Init(m_player);
			m_stage.SetPlayer(m_player);
			nsSound::SetLoop(EnSound::Bgm, true);

			const char* goalText = "GOAL";
			m_goalText.Init(static_cast<int>(strlen(goalText)), GOAL_TEXT_HEIGHT, HudText::Align::Center);
			m_goalText.SetText(goalText);
			m_goalText.SetPosition(Vector2(0.0f, GOAL_FONT_Y));
			m_goalText.SetColor(Vector4(0.0f, 0.9f, 1.0f, 1.0f));
			return true;
		});

	// ポーズのメニューと、黒くする係（黒から戻すのは Begin）
	m_loadTasks.push_back([this]()
		{
			LoadTimer timer("Hud::PauseMenu");
			m_pauseMenu.Init();
			m_fade = FindGO<Fade>(Fade::NAME);
			return true;
		});

	// 敵を出す場所（敵はまだ出さない）
	m_loadTasks.push_back([this]()
		{
			LoadTimer timer("enemySpawner::Init");
			m_enemySpawner.Init(m_player);
			return true;
		});

	// 敵を数体ずつ出す（全部出し終わったら true。それまでは false で、続きは次のフレーム）
	m_loadTasks.push_back([this]()
		{
			LoadTimer timer("enemySpawner::SpawnNext");
			return m_enemySpawner.SpawnNext();
		});
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
	// 遊び始めるまで（タイトルがトンネルを見せている間）は、何もしない
	if (!m_isBegun)
	{
		return;
	}

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
