#pragma once

#include "Level3DRender/LevelRender.h"
#include "input/InputManager.h"
#include "debug/DebugManager.h"
#include "controller/PlayerController.h"

class Player;
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
	Player* m_player = nullptr;
	GameCamera* m_gameCamera = nullptr;
	BulletManager* m_bulletManager = nullptr;

	static constexpr int WALL_NUM = 2;
	static constexpr int ANCHOR_NUM = 8;

	Anchor* m_anchor[ANCHOR_NUM] = {};
	InputManager m_input;
	ModelRender m_floorRender;		// 仮の床（ステージができたら置き換える）
	BoxCollider m_floorCollider;	
	RigidBody   m_floorBody;	
	ModelRender m_wallRender[WALL_NUM];		//仮の壁（ステージができたら置き換える）
	BoxCollider m_wallCollider[WALL_NUM];
	RigidBody m_wallBody[WALL_NUM];
	SpriteRender m_crosshairRender;		///< 仮の照準（HUD ができたら置き換える）
	DebugManager m_debugManager;		///< デバッグ画面（ImGui のパネル。F1 で隠せる）
	PlayerController m_playerController;	///< 入力からプレイヤーのコマンドを作る
};

