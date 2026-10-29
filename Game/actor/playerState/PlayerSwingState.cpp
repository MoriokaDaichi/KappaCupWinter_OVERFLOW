#include "stdafx.h"
#include "actor/playerState/PlayerSwingState.h"

#include "actor/BattleCharacter.h"
#include "gimmick/Anchor.h"
#include "grapple/GrappleWire.h"

void PlayerSwingState::Enter()
{
	m_owner->GetGrapple().GetWire().SetVisible(true);

	m_swingAnchor = m_owner->GetLockedAnchor();

	//まだつかまない。ワイヤーの先を手元から飛ばし、届いたら HookAnchor でつかむ
	m_wireTipLength = 0.0f;
	m_isHooked = false;
	m_owner->ResetJumpCount();
}

void PlayerSwingState::Update()
{
	if (m_owner->IsGrappleRelease())
	{
		ReleaseBoost();
		m_owner->ChangeState(EnPlayerState::Air);
		return;
	}

	if (m_owner->IsJumpTrigger())
	{
		ReleaseBoost();
		m_owner->Jump();
		m_owner->ChangeState(EnPlayerState::Air);
		return;
	}

	//ワイヤーが飛んでいる途中は地上にいてもいいので、つかんでからだけ着地を見る
	if (m_isHooked && m_owner->IsOnGround() && m_owner->IsFalling())
	{
		if (m_owner->IsMoveInput())
		{
			m_owner->ChangeState(EnPlayerState::Run);
		}
		else
		{
			m_owner->ChangeState(EnPlayerState::Idle);
		}
		return;
	}

	Move();
}

void PlayerSwingState::Exit()
{
	m_swingAnchor = nullptr;

	// 離したときの速さを上乗せに残す（空中で減速しないように）
	if (m_isHooked)
	{
		Vector3 horizontal = m_owner->GetMoveSpeed();
		horizontal.y = 0.0f;
		m_owner->GetSpeedSystem().RaiseBonusTo(horizontal.Length() - m_owner->GetSpeedSystem().GetBaseSpeed());
	}

	m_isHooked = false;
	m_owner->GetGrapple().GetWire().SetVisible(false);
}

void PlayerSwingState::Move()
{
	// ワイヤーの先を渡す。つかんだ後はどこまでも届く長さにして、手からアンカーまで全部描く
	const float tipLength = m_isHooked ? WIRE_HOOKED_LENGTH : m_wireTipLength;
	m_owner->GetGrapple().GetWire().SetTarget(m_swingAnchor->GetPosition(), tipLength);

	if (!m_isHooked)
	{
		ShootWire();
		return;
	}

	m_owner->ApplyGravity();

	const Vector3 toPlayer = m_owner->GetChestPosition() - m_swingAnchor->GetPosition();
	const float distance = toPlayer.Length();

	if (distance < SWING_MIN_DISTANCE)
	{
		return;
	}

	Vector3 toPlayerDir = toPlayer;
	toPlayerDir.Normalize();

	SwingPump(toPlayerDir);

	// ロープの外へ逃げる速さを消す（これで円を描く）
	const float outSpeed = m_owner->GetMoveSpeed().Dot(toPlayerDir);
	if (distance >= m_ropeLength && outSpeed > 0.0f)
	{
		m_owner->AddMoveSpeed(toPlayerDir * -outSpeed);
	}

	// 巻き上げ: 最短の長さに近いほどゆっくり引き寄せる（着いた勢いで飛び出さないように）
	float reelSpeed = 0.0f;
	if (m_ropeLength > m_param.minRopeLength)
	{
		reelSpeed = min(m_param.reelSpeed, (distance - m_param.minRopeLength) * m_param.reelEase);

		// 近づいた分だけロープも縮める（伸びることはない）
		m_ropeLength = max(min(m_ropeLength, distance), m_param.minRopeLength);
	}

	// ロープが伸びた分を戻すのに必要な、内向きの速さ
	const float pullSpeed = max(distance - m_ropeLength, 0.0f) * m_param.pullRate;

	// 内向きの速さが足りないときだけ足す（毎フレーム足し続けると、ばねのように跳ね飛ばされる）
	// 横の速さ（振り子の勢い）はそのまま残すので、引き寄せられながら振れる
	const float needSpeed = max(reelSpeed, pullSpeed);
	const float inSpeed = -m_owner->GetMoveSpeed().Dot(toPlayerDir);
	if (inSpeed < needSpeed)
	{
		m_owner->AddMoveSpeed(toPlayerDir * -(needSpeed - inSpeed));
	}

	ClampSwingSpeed();

	Vector3 horizontal = m_owner->GetMoveSpeed();
	horizontal.y = 0.0f;

	if (horizontal.LengthSq() > m_param.faceMinSpeed * m_param.faceMinSpeed)
	{
		m_owner->FaceDirection(horizontal);
	}

	UpdateTilt();
}

void PlayerSwingState::ShootWire()
{
	const Vector3 origin = m_owner->GetChestPosition();
	const Vector3 toAnchor = m_swingAnchor->GetPosition() - origin;
	const float distance = toAnchor.Length();

	m_wireTipLength += m_param.wireShootSpeed * g_gameTime->GetFrameDeltaTime();

	//届いた
	if (m_wireTipLength >= distance)
	{
		HookAnchor(distance);
		return;
	}

	// ワイヤーの見た目は、骨が動いた後の OnBodyUpdated で手から描く
	//まだつかんでいないので、いつもどおり動く（床の上では重力をかけない）
	if (m_owner->IsOnGround())
	{
		m_owner->Move();
	}
	else
	{
		m_owner->AirMove();
		m_owner->ApplyGravity();
	}
}

void PlayerSwingState::HookAnchor(const float ropeLength)
{
	m_isHooked = true;
	m_ropeLength = ropeLength;

	//地上でつかんだら少し浮かせる（すぐ「着地した」と判定されないように）
	if (m_owner->IsOnGround())
	{
		m_owner->SetVerticalSpeed(m_param.startLift);
	}
}

void PlayerSwingState::SwingPump(const Vector3& ropeDir)
{
	if (!m_owner->IsMoveInput())
	{
		return;
	}

	Vector3 input = m_owner->GetMoveDirection();
	input -= ropeDir * input.Dot(ropeDir);

	if (input.LengthSq() < PUMP_MIN_INPUT)
	{
		return;
	}

	input.Normalize();
	m_owner->AddMoveSpeed(input * m_param.pumpAccel * g_gameTime->GetFrameDeltaTime());
}

void PlayerSwingState::ClampSwingSpeed()
{
	Vector3 moveSpeed = m_owner->GetMoveSpeed();

	if (moveSpeed.LengthSq() > m_param.maxSpeed * m_param.maxSpeed)
	{
		moveSpeed.Normalize();
		moveSpeed *= m_param.maxSpeed;
	}

	m_owner->SetMoveSpeed(moveSpeed);
}

void PlayerSwingState::ReleaseBoost()
{
	if (!m_isHooked)
	{
		return;
	}

	if (m_owner->GetMoveSpeed().LengthSq() < m_param.releaseMinSpeed * m_param.releaseMinSpeed)
	{
		return;
	}

	Vector3 speed = m_owner->GetMoveSpeed();
	speed.Normalize();
	m_owner->AddMoveSpeed(speed * m_param.releaseBoost);
	m_owner->AddMoveSpeed(Vector3::Up * m_param.releaseLift);
}

void PlayerSwingState::UpdateTilt()
{
	// 体の上をアンカーに向ける（ロープにぶら下がって見えるように）。胸を中心に回す
	Quaternion target;
	target.SetRotation(Vector3::Up, m_swingAnchor->GetPosition() - m_owner->GetChestPosition());

	m_owner->SetModelTiltTarget(target, m_bodyParam.chestHeight);
}
