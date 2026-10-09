#pragma once

#include "Level3DRender/LevelRender.h"
#include "InputManager.h"
#include "physics/Gravity.h"

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
	InputManager m_input;
	BoxCollider m_floorCollider;	// 仮の床（ステージができたら置き換える）
	RigidBody   m_floorBody;
	CharacterController m_charaCon;	// 仮のキャラクター（重力の確認用。Player ができたら移す）
	Gravity m_gravity;		// 重力
	Vector3 m_moveSpeed;		// 移動速度
};

