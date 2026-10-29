#pragma once

#include "Level3DRender/LevelRender.h"
#include "debug/DebugManager.h"
#include "controller/PlayerController.h"
#include "controller/AIController.h"

class BattleCharacter;
class GameCamera;
class Anchor;
class BulletManager;

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
	BattleCharacter* m_enemy = nullptr;		///< 仮: 歩く敵（AIController で動かす BattleCharacter。敵のスポーンができたら置き換える）
	GameCamera* m_gameCamera = nullptr;
	BulletManager* m_bulletManager = nullptr;

	static constexpr int WALL_NUM = 2;
	static constexpr int ANCHOR_NUM = 8;
	static constexpr float FLOOR_WIDTH = 20000.0f;		///< 仮の床の幅と奥行き
	static constexpr float FLOOR_THICKNESS = 10.0f;		///< 仮の床の厚さ
	static constexpr float DEBUG_MODEL_SIZE = 100.0f;	///< 仮の壁のモデル（debug_wall.tkm）の 1 辺の長さ。これで割って何倍に広げるかを出す
	static constexpr float CROSSHAIR_SIZE = 48.0f;		///< 仮の照準の画像の大きさ（縦横）

	Anchor* m_anchor[ANCHOR_NUM] = {};
	ModelRender m_floorRender;		// 仮の床（ステージができたら置き換える）
	BoxCollider m_floorCollider;	
	RigidBody   m_floorBody;	
	ModelRender m_wallRender[WALL_NUM];		//仮の壁（ステージができたら置き換える）
	BoxCollider m_wallCollider[WALL_NUM];
	RigidBody m_wallBody[WALL_NUM];
	SpriteRender m_crosshairRender;		///< 仮の照準（HUD ができたら置き換える）
	DebugManager m_debugManager;		///< デバッグ画面（ImGui のパネル。F1 で隠せる）
	PlayerController m_playerController;	///< 入力からプレイヤーのコマンドを作る
	AIController m_enemyController;			///< 敵のコマンドを作る（プレイヤーの方へ歩く）
	static constexpr float START_HEIGHT = 300.0f;		///< プレイヤーと敵を置く高さ（床の少し上から落とす）
};

