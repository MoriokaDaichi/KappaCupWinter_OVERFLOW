#pragma once

#include "Level3DRender/LevelRender.h"
#include "InputManager.h"

class Player;

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

	InputManager m_input;
	BoxCollider m_floorCollider;	// 仮の床（ステージができたら置き換える）
	RigidBody   m_floorBody;	
};

