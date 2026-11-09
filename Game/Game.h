#pragma once

#include "Level3DRender/LevelRender.h"
#include "debug/DebugManager.h"
#include "controller/PlayerController.h"
#include "actor/EnemySpawner.h"
#include "hud/Hud.h"
#include "hud/LockOnHud.h"
#include "stage/Stage.h"
#include "stage/Checkpoint.h"
#include "gimmick/GiantFirewall.h"

class BattleCharacter;
class GameCamera;
class BulletSystem;

class Game : public IGameObject
{
public:
	Game() {}
	~Game();
	bool Start();
	void Update();
	void Render(RenderContext& rc);

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

	Stage m_stage;						///< stage.json の箱とアンカー
	Hud m_hud;							///< タイム・チェックポイント・照準・スピードメーター・グラップルの角カッコ
	LockOnHud m_lockOnHud;				///< マルチロックオンのドーナツ
	DebugManager m_debugManager;		///< デバッグ画面（ImGui のパネル。F1 で隠せる）
	PlayerController m_playerController;	///< 入力からプレイヤーのコマンドを作る
	EnemySpawner m_enemySpawner;			///< spawn.json の場所に敵を出す
	Checkpoint m_checkpoint;			///< 落下時の復帰地点用
	GiantFirewall m_giantFirewall;		///< ゴール地点の巨大FW用
	HudText m_goalText;					///< 巨大ファイアウォールを突破したときに出す「GOAL」の文字（リザルト画面ができるまでの仮）
};

