#pragma once

#include "Level3DRender/LevelRender.h"
#include "InputManager.h"

class Player;

class Game : public IGameObject
{
public:
	Game() {}
	~Game() {}
	bool Start();
	void Update();
	void Render(RenderContext& rc);

private:
	ModelRender m_modelRender;
	Vector3 m_pos;
	InputManager m_input;
	DebugGuiPanel m_debugPanel;
};

