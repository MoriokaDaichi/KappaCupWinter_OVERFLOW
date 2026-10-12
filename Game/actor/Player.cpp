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
		ApplyGravity();
	}

	Vector3 position = m_charaCon.Execute(m_moveSpeed, g_gameTime->GetFrameDeltaTime());

	m_modelRender.SetRotation(m_rotation);
	m_modelRender.SetPosition(position);
	m_modelRender.Update();
}

void Player::Move()
{
	Vector3 moveDir = GetMoveDirection();

	//速度に入れる
	m_moveSpeed.x = moveDir.x * MOVE_SPEED;
	m_moveSpeed.z = moveDir.z * MOVE_SPEED;

	//スティックを倒しているときだけ向きを変える（離したときは今の向きを保つ）
	if (moveDir.LengthSq() > MOVE_INPUT_THRESHOLD)
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

bool Player::IsJumpTrigger() const
{
	return m_input->IsTrigger(EnInputAction::Jump);
}

bool Player::IsMoveInput() const
{
	Vector2 stick = m_input->GetMove();
	float length = stick.x * stick.x + stick.y * stick.y;
	return length > MOVE_INPUT_THRESHOLD;
}

void Player::ChangeState(EnPlayerState id)
{
	m_stateMachine.ChangeState(id);
}

void Player::ResetJumpCount()
{
	m_jumpCount = 0;
}

void Player::StopHorizontal()
{
	m_moveSpeed.x = 0.0f;
	m_moveSpeed.z = 0.0f;
}

void Player::ApplyGravity()
{
	m_gravity.Apply(m_moveSpeed);
}

bool Player::IsFalling() const
{
	return m_moveSpeed.y <= 0.0f;
}

void Player::ConsumeGroundJump()
{
	if (m_jumpCount == 0)
	{
		m_jumpCount = 1;
	}
}

void Player::AirMove()
{
	if (!IsMoveInput())
	{
		return;
	}

	Vector3 moveDir = GetMoveDirection();
	Vector3 target = moveDir * MOVE_SPEED;

	float t = AIR_CONTROL * g_gameTime->GetFrameDeltaTime();
	if (t > 1.0f)
	{
		t = 1.0f;
	}

	//今の速さを目標の速さに t の割合だけ近づける
	m_moveSpeed.x += (target.x - m_moveSpeed.x) * t;
	m_moveSpeed.z += (target.z - m_moveSpeed.z) * t;

	m_rotation.SetRotationYFromDirectionXZ(moveDir);
}

Vector3 Player::GetMoveDirection() const
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
	return forward * stick.y + right * stick.x;
}

void Player::Render(RenderContext& rc)
{
	m_modelRender.Draw(rc);
}
