#pragma once

#include "Level3DRender/LevelRender.h"
#include "debug/DebugManager.h"
#include "controller/PlayerController.h"
#include "actor/EnemySpawner.h"
#include "hud/Hud.h"
#include "hud/LevelUpFlash.h"
#include "hud/DefeatFlash.h"
#include "hud/PauseMenu.h"
#include "hud/LockOnHud.h"
#include "stage/Stage.h"
#include "stage/Checkpoint.h"
#include "gimmick/GiantFirewall.h"
#include <chrono>
#include <functional>

class BattleCharacter;
class GameCamera;
class BulletSystem;
class Fade;

class Game : public IGameObject
{
public:
	Game() {}
	~Game();
	bool Start();
	void Update();
	void Render(RenderContext& rc);

public:
	/** 読み込みが全部終わったか（Start が最後の段まで進み、敵の Start も終わった）。タイトルが毎フレーム聞く */
	bool IsReady() const { return IsStart(); }
	/**
	 * 遊び始める。読み込みの間止めていた時間を戻し、黒から戻す
	 * タイトルが画面を黒くし終えてから、1 回だけ呼ぶ
	 */
	void Begin();

private:
	/** 最初からやり直す（プレイヤー・チェックポイント・タイム・敵・巨大ファイアウォールを最初に戻す。デバッグ画面か Back ボタンから） */
	void Reset();
	/** ポーズにする・やめる。ポーズの間はゲームの時間を 0 にして（g_gameTime）、プレイヤーへの入力を止める */
	void SetPaused(const bool isPaused);
	/** ポーズのメニューを動かし、選んだものに合わせて、再開・チェックポイントへ戻る・タイトルへ戻る */
	void UpdatePause();
	/** 画面を黒くし始める。黒くなったら next のシーンへ移る（ゴールの後はリザルト、ポーズからはタイトル） */
	void StartLeave(const int next);
	/** 黒くなるのを待って、次のシーンを作り、自分を消す */
	void UpdateLeave();
	/** 足音とスイングの音を止める（ポーズの間と、シーンを抜けるとき） */
	void StopLoopSounds();
	/**
	 * 読み込みでやることを、前から順に m_loadTasks に並べる（最初の Start で 1 回だけ呼ぶ）
	 * 作業は「終わったら true」を返す。何回かに分ける作業（敵を 1 体ずつ出すなど）は、残りがあれば false を返す
	 */
	void BuildLoadTasks();

private:
	BattleCharacter* m_player = nullptr;	///< プレイヤー（PlayerController で動かす BattleCharacter）
	GameCamera* m_gameCamera = nullptr;
	BulletSystem* m_bulletSystem = nullptr;
	BulletSystem* m_enemyBulletSystem = nullptr;

	static constexpr float SWING_MIN_VOLUME_RATE = 0.3f;	///< スイングの音の、止まりかけのときの音量の割合（速さが 0 でも少し鳴らす）
	static constexpr float SWING_MIN_PITCH = 0.85f;			///< スイングの音の、止まりかけのときの高さの割合
	static constexpr float SWING_MAX_PITCH = 1.3f;			///< スイングの音の、いちばん速いときの高さの割合
	static constexpr float GOAL_TEXT_HEIGHT = 110.0f;	///< 「GOAL」の文字の高さ（ピクセル）
	static constexpr float GOAL_FONT_Y = 100.0f;		///< 画面の真ん中からの高さ。真ん中ぴったりだと照準と重なるので、少し上にする
	static constexpr float GOAL_TO_RESULT_TIME = 3.0f;	///< 巨大ファイアウォールを突破してから、リザルトへ移り始めるまでの秒数（GOAL の文字を見せる）
	static constexpr float FADE_TIME = 0.5f;			///< 黒くする・戻す秒数
	static constexpr int LEAVE_NONE = 0;				///< 抜けない（遊んでいる）
	static constexpr int LEAVE_TO_RESULT = 1;			///< リザルトへ移る
	static constexpr int LEAVE_TO_TITLE = 2;			///< タイトルへ戻る
	static constexpr float LOAD_BUDGET_SEC = 0.03f;	///< 読み込みで 1 フレームに使ってよい秒数（超えたら次のフレームへ。ダイブの絵を止めないため）
	static constexpr float LOAD_BUSY_SEC = 0.05f;	///< 前の Start からこれ以上たっていたら、このフレームはほかの Start でもう重いとみなし、作業を休む

	Stage m_stage;						///< stage.json の箱とアンカー
	LevelUpFlash m_levelUpFlash;		///< 速度 Lv が上がった瞬間に画面を光らせる
	DefeatFlash m_defeatFlash;			///< 敵を倒した瞬間に画面を薄く光らせる
	Hud m_hud;							///< タイム・チェックポイント・照準・スピードメーター・グラップルの角カッコ
	LockOnHud m_lockOnHud;				///< マルチロックオンのドーナツ
	DebugManager m_debugManager;		///< デバッグ画面（ImGui のパネル。F1 で隠せる）
	PlayerController m_playerController;	///< 入力からプレイヤーのコマンドを作る
	EnemySpawner m_enemySpawner;			///< spawn.json の場所に敵を出す
	Checkpoint m_checkpoint;			///< 落下時の復帰地点用
	GiantFirewall m_giantFirewall;		///< ゴール地点の巨大FW用
	HudText m_goalText;					///< 巨大ファイアウォールを突破したときに出す「GOAL」の文字
	PauseMenu m_pauseMenu;				///< ポーズのメニュー
	Fade* m_fade = nullptr;				///< 画面を黒くする係（main が作ったものを借りる）
	bool m_isPaused = false;			///< ポーズ中か
	bool m_isInputRestorePending = false;	///< ポーズを閉じた次のフレームで、プレイヤーへの入力を戻すか（閉じた A でジャンプしないように）
	int m_leaveTo = LEAVE_NONE;			///< 抜けて移る先（LEAVE_ の値）
	float m_goalTimer = 0.0f;			///< 突破してからの秒数
	std::vector<std::function<bool()>> m_loadTasks;	///< 読み込みでやることの列（前から順にやる。BuildLoadTasks で作る）
	int m_loadTaskIndex = 0;						///< 次にやる m_loadTasks の番号
	std::chrono::steady_clock::time_point m_lastLoadTime;	///< 前に Start を抜けた時刻（このフレームがもう重いかを見るため）
	bool m_isLoadRested = false;					///< 前のフレームで作業を休んだか（2 フレーム続けては休まない。休み続けて読み込みが進まなくならないように）
	bool m_isBegun = false;				///< Begin() が呼ばれたか（それまでは Update も Render もしない）
};

