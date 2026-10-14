#pragma once

#include "Level3DRender/LevelRender.h"
#include "InputManager.h"

class Player;
class GameCamera;

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

	InputManager m_input;
	ModelRender m_floorRender;		// 仮の床（ステージができたら置き換える）
	BoxCollider m_floorCollider;	
	RigidBody   m_floorBody;	
	ModelRender m_wallRender[WALL_NUM];		//仮の壁（ステージができたら置き換える）
	BoxCollider m_wallCollider[WALL_NUM];
	RigidBody m_wallBody[WALL_NUM];
};

