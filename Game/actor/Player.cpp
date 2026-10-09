#include "stdafx.h"
#include "actor/Player.h"

#include "InputManager.h"

Player::Player() : m_stateMachine(this)
{
}

Player::~Player()
{
}

bool Player::Start()
{
	m_modelRender.Init("Assets/modelData/unityChan.tkm");

	m_charaCon.Init(25.0f, 75.0f, Vector3(0.0f, 300.0f, 0.0f));

	return true;
}

void Player::Update()
{
	Move();

	//着地したらジャンプの回数を戻す。ジャンプせずに足場から落ちたときは、1回分を使ったことにする
	if (m_charaCon.IsOnGround())
	{
		m_jumpCount = 0;
	}
	else if (m_jumpCount == 0)
	{
		m_jumpCount = 1;
	}

	//ジャンプ。= で入れるので、落ちている途中に跳んでも同じ高さだけ上がる
	if (m_input->IsTrigger(EnInputAction::Jump) && CanJump())
	{
		Jump();
	}

	//床の上では重力をかけない（かけると毎フレーム床にめり込んで、ガタガタする）
	if (!m_charaCon.IsOnGround())
	{
		m_gravity.Apply(m_moveSpeed);
	}

	Vector3 position = m_charaCon.Execute(m_moveSpeed, g_gameTime->GetFrameDeltaTime());

	m_modelRender.SetRotation(m_rotation);
	m_modelRender.SetPosition(position);
	m_modelRender.Update();
}

void Player::Move()
{
	//スティックの入力
	Vector2 stick = m_input->GetMove();

	//カメラの前向き・右向き（地面と平行にする）
	Vector3 forward = g_camera3D->GetForward();
	forward.y = 0.0f;
	forward.Normalize();
	Vector3 right = g_camera3D->GetRight();
	right.y = 0.0f;
	right.Normalize();

	//移動方向の決定
	Vector3 moveDir = forward * stick.y + right * stick.x;

	//速度に入れる
	m_moveSpeed.x = moveDir.x * MOVE_SPEED;
	m_moveSpeed.z = moveDir.z * MOVE_SPEED;

	//スティックを倒しているときだけ向きを変える（離したときは今の向きを保つ）
	if (moveDir.LengthSq() > 0.001f)
	{
		m_rotation.SetRotationYFromDirectionXZ(moveDir);
	}
}

void Player::Jump()
{
	// = で入れるので、落ちている途中に跳んでも同じ高さだけ上がる
	m_moveSpeed.y = JUMP_SPEED;
	m_jumpCount += 1;
}

void Player::Render(RenderContext& rc)
{
	m_modelRender.Draw(rc);
}
