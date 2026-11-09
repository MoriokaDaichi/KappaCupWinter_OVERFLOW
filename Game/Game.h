#pragma once

#include "Level3DRender/LevelRender.h"
#include "debug/DebugManager.h"
#include "controller/PlayerController.h"
#include "actor/EnemySpawner.h"
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

	static constexpr float CROSSHAIR_SIZE = 48.0f;		///< 仮の照準の画像の大きさ（縦横）

	Stage m_stage;						///< stage.json の箱とアンカー
	SpriteRender m_crosshairRender;		///< 仮の照準（HUD ができたら置き換える）
	LockOnHud m_lockOnHud;				///< マルチロックオンのドーナツ
	DebugManager m_debugManager;		///< デバッグ画面（ImGui のパネル。F1 で隠せる）
	PlayerController m_playerController;	///< 入力からプレイヤーのコマンドを作る
	EnemySpawner m_enemySpawner;			///< spawn.json の場所に敵を出す
	Checkpoint m_checkpoint;			///< 落下時の復帰地点用
	GiantFirewall m_giantFirewall;		///< ゴール地点の巨大FW用
};

