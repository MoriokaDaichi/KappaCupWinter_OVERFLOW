#pragma once

#include "Level3DRender/LevelRender.h"
#include "input/InputManager.h"

class Player;
class GameCamera;
class Anchor;

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
	bool m_isReloadKeyDown = false;		///< 前のフレームで F5 を押していたか（押した瞬間だけ読み直すため）
};

