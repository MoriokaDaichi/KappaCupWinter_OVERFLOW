#include "stdafx.h"
#include "actor/Player.h"

#include "input/InputManager.h"
#include "actor/playerState/PlayerIdleState.h"
#include "actor/playerState/PlayerRunState.h"
#include "actor/playerState/PlayerAirState.h"
#include "actor/playerState/PlayerWallRunState.h"
#include "actor/playerState/PlayerSwingState.h"
#include "collision/RayCast.h"
#include "gimmick/Anchor.h"
#include "grapple/GrappleWire.h"

Player::Player() : m_stateMachine(this)
{
}

Player::~Player()
{
	DeleteGO(m_grappleWire);
}

bool Player::Start()
{
	InitCharacter("Assets/modelData/unityChan.tkm", 25.0f, 75.0f, Vector3(0.0f, 300.0f, 0.0f));

	m_stateMachine.AddState<PlayerIdleState>(EnPlayerState::Idle);
	m_stateMachine.AddState<PlayerRunState>(EnPlayerState::Run);
	m_stateMachine.AddState<PlayerAirState>(EnPlayerState::Air);
	m_stateMachine.AddState<PlayerWallRunState>(EnPlayerState::WallRun);
	m_stateMachine.AddState<PlayerSwingState>(EnPlayerState::Swing);

	ChangeState(EnPlayerState::Idle);

	m_grappleWire = NewGO<GrappleWire>(0, "grappleWire");

	return true;
}

void Player::Move()
{
	Vector3 moveDir = GetMoveDirection();

	//速度に入れる
	ApproachHorizontal(moveDir * MOVE_SPEED, GROUND_ACCEL);

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
	Vector3 moveDir = forward * stick.y + right * stick.x;

	//キーボードだと斜めの長さが 0.7 くらいになるので、長さを 1 にそろえる（入力がないときは 0 のまま）
	if (moveDir.LengthSq() > MOVE_INPUT_THRESHOLD)
	{
		moveDir.Normalize();
	}
	return moveDir;
}

bool Player::FindWall()
{
	Vector3 right = Vector3::Right;
	m_rotation.Apply(right);

	Vector3 from = GetPosition();
	from.y += WALL_CHECK_HEIGHT;

	const Vector3 dirs[] = { right, right * -1.0f };
	for (const Vector3& dir : dirs)
	{
		Vector3 to = from + dir * WALL_CHECK_LENGTH;
		RayHitInfo hit = nsCollision::RayCast(from, to);

		if (!hit.isHit)
		{
			continue;
		}

		if (fabsf(hit.normal.y) >= WALL_NORMAL_Y_LIMIT)
		{
			continue;
		}

		m_wallNormal = hit.normal;
		return true;
	}

	return false;
}

bool Player::CanStartWallRun() const
{
	Vector3 horizontal = GetMoveSpeed();
	horizontal.y = 0.0f;
	float speed = horizontal.Length();

	return IsMoveInput() && speed >= WALLRUN_MIN_SPEED && m_reattachTimer <= 0.0f;
}

void Player::StartWallRun()
{
	m_wallRunTimer = 0.0f;
	m_moveSpeed.y = 0.0f;

	m_gravity.SetScale(WALLRUN_GRAVITY_SCALE);
	ResetJumpCount();
}

void Player::WallRunMove()
{
	Vector3 along = Cross(m_wallNormal, Vector3::Up);
	along.Normalize();

	if (along.Dot(m_moveSpeed) < 0.0f)
	{
		along *= -1.0f;
	}

	Vector3 horizontalSpeed = along * WALLRUN_SPEED + m_wallNormal * -WALLRUN_STICK_SPEED;

	m_moveSpeed.x = horizontalSpeed.x;
	m_moveSpeed.z = horizontalSpeed.z;

	m_rotation.SetRotationYFromDirectionXZ(along);
	m_wallRunTimer += g_gameTime->GetFrameDeltaTime();
	if (!IsWallRunTimeOver())
	{
		m_moveSpeed.y = 0.0f;
	}
	else
	{
		ApplyGravity();
	}
}

void Player::WallJump()
{
	m_moveSpeed += m_wallNormal * WALL_JUMP_SPEED_OUT;
	m_moveSpeed.y = WALL_JUMP_SPEED_UP;
	m_jumpCount += 1;
}

void Player::EndWallRun()
{
	m_gravity.SetScale(1.0f);
	m_reattachTimer = WALL_REATTACH_TIME;
}

bool Player::IsWallRunTimeOver() const
{
	return m_wallRunTimer >= WALLRUN_MAX_TIME;
}

void Player::ApproachHorizontal(const Vector3& target, float rate)
{
	Vector3 diff = target - m_moveSpeed;
	diff.y = 0.0f;

	float step = rate * g_gameTime->GetFrameDeltaTime();

	if (diff.Length() <= step)
	{
		m_moveSpeed.x = target.x;
		m_moveSpeed.z = target.z;
		return;
	}

	diff.Normalize();
	m_moveSpeed.x += diff.x * step;
	m_moveSpeed.z += diff.z * step;
}

void Player::Decelerate()
{
	ApproachHorizontal(Vector3::Zero, GROUND_DECEL);
}

bool Player::IsGrappleTrigger() const
{
	return m_input->IsTrigger(EnInputAction::Grapple);
}

bool Player::IsGrappleRelease() const
{
	return m_input->IsRelease(EnInputAction::Grapple);
}

void Player::StartSwing()
{
	m_grappleWire->SetVisible(true);

	m_swingAnchor = m_lockedAnchor;

	//まだつかまない。ワイヤーの先を手元から飛ばし、届いたら HookAnchor でつかむ
	m_wireTipLength = 0.0f;
	m_isHooked = false;
	ResetJumpCount();
}

void Player::SwingMove()
{
	if (!m_isHooked)
	{
		ShootWire();
		return;
	}

	ApplyGravity();

	Vector3 toPlayer = GetRopeOrigin() - m_swingAnchor->GetPosition();
	float distance = toPlayer.Length();

	m_grappleWire->SetEnds(GetRopeOrigin(), m_swingAnchor->GetPosition());

	if (distance < SWING_MIN_DISTANCE)
	{
		return;
	}

	Vector3 toPlayerDir = toPlayer;
	toPlayerDir.Normalize();

	// ロープの外へ逃げる速さを消す（これで円を描く）
	float outSpeed = m_moveSpeed.Dot(toPlayerDir);
	if (distance >= m_ropeLength && outSpeed > 0.0f)
	{
		m_moveSpeed -= toPlayerDir * outSpeed;
	}

	// ロープが伸びた分だけ、内向きに引き戻す
	float stretch = distance - m_ropeLength;
	if (stretch > 0.0f)
	{
		m_moveSpeed += toPlayerDir * -(stretch * SWING_PULL_RATE);
	}
}

void Player::EndSwing()
{
	m_swingAnchor = nullptr;
	m_isHooked = false;
	m_grappleWire->SetVisible(false);
}

void Player::ShootWire()
{
	Vector3 origin = GetRopeOrigin();
	Vector3 toAnchor = m_swingAnchor->GetPosition() - origin;
	float distance = toAnchor.Length();

	m_wireTipLength += WIRE_SHOOT_SPEED * g_gameTime->GetFrameDeltaTime();

	//届いた
	if (m_wireTipLength >= distance)
	{
		m_grappleWire->SetEnds(origin, m_swingAnchor->GetPosition());
		HookAnchor(distance);
		return;
	}

	//飛んでいる途中は、手元から先までだけ描く
	toAnchor.Normalize();
	m_grappleWire->SetEnds(origin, origin + toAnchor * m_wireTipLength);

	//まだつかんでいないので、いつもどおり動く（床の上では重力をかけない）
	if (IsOnGround())
	{
		Move();
	}
	else
	{
		AirMove();
		ApplyGravity();
	}
}

void Player::HookAnchor(float ropeLength)
{
	m_isHooked = true;
	m_ropeLength = ropeLength;

	//地上でつかんだら少し浮かせる（すぐ「着地した」と判定されないように）
	if (IsOnGround())
	{
		m_moveSpeed.y = SWING_START_LIFT;
	}
}

void Player::UpdateAction()
{
	m_reattachTimer -= g_gameTime->GetFrameDeltaTime();
	if (m_reattachTimer < 0.0f)
	{
		m_reattachTimer = 0.0f;
	}

	UpdateLock();

	if (IsGrappleTrigger() && CanStartSwing())
	{
		ChangeState(EnPlayerState::Swing);
	}

	m_stateMachine.Update();
}

void Player::UpdateLock()
{
	Anchor* best = nullptr;
	float bestScore = 0.0f;

	QueryGOs<Anchor>("anchor", [&](Anchor* anchor)
		{
			float score = CalcLockScore(anchor);

			if( score < 0.0f)
			{
				return true;
			}

			if (anchor == m_lockedAnchor)
			{
				score += LOCK_KEEP_BONUS;
			}

			if (best == nullptr || score > bestScore)
			{
				best = anchor;
				bestScore = score;
			}

			return true;
		});

	if (best == m_lockedAnchor)
	{
		return;
	}

	if (m_lockedAnchor != nullptr)
	{
		m_lockedAnchor->SetLocked(false);
	}

	if (best != nullptr)
	{
		best->SetLocked(true);
	}

	m_lockedAnchor = best;
}

float Player::CalcLockScore(const Anchor* anchor) const
{
	Vector3 toAnchor = anchor->GetPosition() - GetPosition();
	float distance = toAnchor.Length();
	if (distance > LOCK_MAX_DISTANCE)
	{
		return -1.0f;
	}

	Vector3 camToAnchor = anchor->GetPosition() - g_camera3D->GetPosition();
	camToAnchor.Normalize();
	float dot = g_camera3D->GetForward().Dot(camToAnchor);
	if (dot < cosf(Math::DegToRad(LOCK_MAX_ANGLE)))
	{
		return -1.0f;
	}

	float angle = Math::RadToDeg(acosf(min(dot, 1.0f)));

	Vector3 from = GetPosition();
	from.y += WALL_CHECK_HEIGHT;		// 胸の高さから飛ばす（足元だと床に当たる）
	if (nsCollision::RayCast(from, anchor->GetPosition()).isHit)
	{
		return -1.0f;
	}

	float angleScore = 1.0f - angle / LOCK_MAX_ANGLE;
	float distanceScore = 1.0f - distance / LOCK_MAX_DISTANCE;

	return angleScore * LOCK_ANGLE_WEIGHT + distanceScore * LOCK_DISTANCE_WEIGHT;
}

Vector3 Player::GetRopeOrigin() const
{
	Vector3 position = GetPosition();
	position.y += WALL_CHECK_HEIGHT;

	return position;
}
