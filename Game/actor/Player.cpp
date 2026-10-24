#include "stdafx.h"
#include "actor/Player.h"

#include "controller/IController.h"
#include "actor/playerState/PlayerIdleState.h"
#include "actor/playerState/PlayerRunState.h"
#include "actor/playerState/PlayerAirState.h"
#include "actor/playerState/PlayerWallRunState.h"
#include "actor/playerState/PlayerSwingState.h"
#include "collision/RayCast.h"
#include "gimmick/Anchor.h"
#include "grapple/GrappleWire.h"
#include "weapon/BulletManager.h"

Player::Player() : m_stateMachine(this)
{
}

Player::~Player()
{
	DeleteGO(m_grappleWire);
}

bool Player::Start()
{
	LoadAnimation();
	InitCharacter("Assets/modelData/player/medea.tkm", m_animClips.data(), static_cast<int>(m_animClips.size()),
		m_param.body.radius, m_param.body.height, Vector3(0.0f, 300.0f, 0.0f));
	m_leftHandBoneId = m_modelRender.FindBoneID(L"mixamorig:LeftHand");
	m_rightHandBoneId = m_modelRender.FindBoneID(L"mixamorig:RightHand");
	m_rightMiddleBoneId = m_modelRender.FindBoneID(L"mixamorig:RightHandMiddle1");

	m_speed.Reset();

	m_stateMachine.AddState<PlayerIdleState>(EnPlayerState::Idle);
	m_stateMachine.AddState<PlayerRunState>(EnPlayerState::Run);
	m_stateMachine.AddState<PlayerAirState>(EnPlayerState::Air);
	m_stateMachine.AddState<PlayerWallRunState>(EnPlayerState::WallRun);
	m_stateMachine.AddState<PlayerSwingState>(EnPlayerState::Swing);

	ChangeState(EnPlayerState::Idle);

	m_grappleWire = NewGO<GrappleWire>(0, "grappleWire");

	m_gun.Init(FindGO<BulletManager>("bulletManager"));
	m_gunModel.Init();

	return true;
}

void Player::Move()
{
	Vector3 moveDir = GetMoveDirection();

	//速度に入れる
	ApproachHorizontal(moveDir * m_speed.GetSpeed(), m_param.move.groundAccel);

	//スティックを倒しているときだけ向きを変える（離したときは今の向きを保つ）
	FaceDirection(moveDir);
}

void Player::Jump()
{
	// = で入れるので、落ちている途中に跳んでも同じ高さだけ上がる
	SetVerticalSpeed(m_param.jump.speed);
	m_jumpCount += 1;
	// 2段目のジャンプでも、踏み切りから流し直す
	PlayAnimation(EnPlayerAnim::JumpUp, true);
}

bool Player::IsJumpTrigger() const
{
	return m_command.jumpTrigger;
}

bool Player::IsMoveInput() const
{
	return m_command.moveDir.LengthSq() > MOVE_INPUT_THRESHOLD;
}

void Player::ChangeState(EnPlayerState id)
{
	m_stateMachine.ChangeState(id);
	PlayStateAnimation(id);
}

void Player::LoadAnimation()
{
	struct ClipInfo
	{
		EnPlayerAnim anim;
		const char* filePath;
		bool isLoop;
	};
	const ClipInfo CLIPS[] = {
		{ EnPlayerAnim::Idle,   "Assets/animData/player/idle.tka",    true },
		{ EnPlayerAnim::Run,    "Assets/animData/player/run.tka",     true },
		{ EnPlayerAnim::JumpUp, "Assets/animData/player/jump_up.tka", false },
		{ EnPlayerAnim::Fall,     "Assets/animData/player/fall_idle.tka", true },
		{ EnPlayerAnim::FallFast, "Assets/animData/player/fall_long.tka", true },
		{ EnPlayerAnim::Land,     "Assets/animData/player/land.tka",      false },
		{ EnPlayerAnim::Swing,  "Assets/animData/player/swing.tka",   true },
	};
	static_assert(std::size(CLIPS) == static_cast<int>(EnPlayerAnim::Num), "CLIPS を EnPlayerAnim に合わせてください");

	for (const ClipInfo& clip : CLIPS)
	{
		AnimationClip& animClip = m_animClips[static_cast<int>(clip.anim)];
		animClip.Load(clip.filePath);
		animClip.SetLoopFlag(clip.isLoop);
	}
}

void Player::PlayStateAnimation(EnPlayerState state)
{
	// 状態ごとのアニメーション（EnPlayerState と同じ順に並べる）。壁走り専用はないので run を使う
	const EnPlayerAnim STATE_TO_ANIM[] = {
		EnPlayerAnim::Idle,		// Idle
		EnPlayerAnim::Run,		// Run
		EnPlayerAnim::Fall,		// Air（実際は SelectAirAnimation で選ぶ）
		EnPlayerAnim::Run,		// WallRun（体を傾けて、壁を走って見せる）
		EnPlayerAnim::Swing,	// Swing
	};
	static_assert(std::size(STATE_TO_ANIM) == static_cast<int>(EnPlayerState::Num), "STATE_TO_ANIM を EnPlayerState に合わせてください");

	if (state == EnPlayerState::Air)
	{
		PlayAnimation(SelectAirAnimation());
		return;
	}

	// 止まって着地したときだけ、着地の動きを流す（走りながらの着地は、足が止まって見えるので run のまま）
	const bool isFromAir = m_currentAnim == EnPlayerAnim::JumpUp || m_currentAnim == EnPlayerAnim::Fall || m_currentAnim == EnPlayerAnim::FallFast;
	if (state == EnPlayerState::Idle && isFromAir && m_airFallSpeed > m_param.jump.landAnimMinSpeed)
	{
		PlayAnimation(EnPlayerAnim::Land);
		return;
	}

	PlayAnimation(STATE_TO_ANIM[static_cast<int>(state)]);
}

void Player::UpdateAnimation()
{
	if (GetCurrentState() == EnPlayerState::Air)
	{
		m_airFallSpeed = max(-m_moveSpeed.y, 0.0f);
		PlayAnimation(SelectAirAnimation());
		return;
	}

	if (m_currentAnim == EnPlayerAnim::Land && !m_modelRender.IsPlayingAnimation())
	{
		PlayAnimation(EnPlayerAnim::Idle);
	}
}

EnPlayerAnim Player::SelectAirAnimation() const
{
	// 踏み切り（Jump で流し始める）は最後まで流す。Jump の後に Air に入るので、ここで消さないようにする
	if (m_currentAnim == EnPlayerAnim::JumpUp && m_modelRender.IsPlayingAnimation())
	{
		return EnPlayerAnim::JumpUp;
	}

	// 高いところから落ちて、速さがついてきた
	if (m_moveSpeed.y < -m_param.jump.fastFallAnimSpeed)
	{
		return EnPlayerAnim::FallFast;
	}

	return EnPlayerAnim::Fall;
}

void Player::PlayAnimation(EnPlayerAnim anim, bool isRestart)
{
	// 壁走り→走るのように同じアニメーションが続くときは、最初から流し直さない
	if (anim == m_currentAnim && !isRestart)
	{
		return;
	}

	m_modelRender.PlayAnimation(static_cast<int>(anim), ANIM_INTERPOLATE_TIME);
	m_currentAnim = anim;
}

Vector3 Player::GetRopeHandPosition() const
{
	if (m_leftHandBoneId < 0 || m_rightHandBoneId < 0)
	{
		return GetChestPosition();
	}

	const Matrix& left = m_modelRender.GetBone(m_leftHandBoneId)->GetWorldMatrix();
	const Matrix& right = m_modelRender.GetBone(m_rightHandBoneId)->GetWorldMatrix();
	return Vector3(
		(left.m[3][0] + right.m[3][0]) * 0.5f,
		(left.m[3][1] + right.m[3][1]) * 0.5f,
		(left.m[3][2] + right.m[3][2]) * 0.5f);
}

void Player::UpdateBodyTilt()
{
	Quaternion target = Quaternion::Identity;
	const EnPlayerState state = GetCurrentState();

	if (state == EnPlayerState::Swing && m_swingAnchor != nullptr && m_isHooked)
	{
		// 体の上をアンカーに向ける（ロープにぶら下がって見えるように）。胸を中心に回す
		target.SetRotation(Vector3::Up, m_swingAnchor->GetPosition() - GetChestPosition());
		m_modelTiltPivotHeight = m_param.body.chestHeight;
	}
	else if (state == EnPlayerState::WallRun)
	{
		// 体の上を壁から離れる向きへ傾け、足を壁につける。壁の向きから決めるので、左右どちらの壁でも同じ
		Vector3 away = m_wallNormal;
		away.y = 0.0f;
		away.Normalize();
		const float angle = Math::DegToRad(m_param.wallRun.tiltAngle);
		target.SetRotation(Vector3::Up, Vector3::Up * cosf(angle) + away * sinf(angle));
		// 回す中心を「傾けたときに足が壁（カプセルの端）に届く高さ」にする
		m_modelTiltPivotHeight = m_param.body.radius / max(sinf(angle), TILT_MIN_SIN);
	}

	// 入るとき・抜けるときに急に傾かないように、少しずつ近づける
	const float rate = min(m_param.body.tiltLerpRate * g_gameTime->GetFrameDeltaTime(), 1.0f);
	m_modelTilt.Slerp(rate, m_modelTilt, target);
}

void Player::UpdateWireEnds()
{
	if (m_swingAnchor == nullptr)
	{
		return;
	}

	const Vector3 hand = GetRopeHandPosition();
	const Vector3 anchorPosition = m_swingAnchor->GetPosition();
	if (m_isHooked)
	{
		m_grappleWire->SetEnds(hand, anchorPosition);
		return;
	}

	// 飛んでいる途中は、手元から先までだけ描く
	Vector3 toAnchor = anchorPosition - hand;
	const float length = min(m_wireTipLength, toAnchor.Length());
	toAnchor.Normalize();
	m_grappleWire->SetEnds(hand, hand + toAnchor * length);
}

void Player::UpdateGunModel()
{
	if (m_rightHandBoneId < 0 || m_rightMiddleBoneId < 0)
	{
		return;
	}

	const Matrix& hand = m_modelRender.GetBone(m_rightHandBoneId)->GetWorldMatrix();
	const Matrix& middle = m_modelRender.GetBone(m_rightMiddleBoneId)->GetWorldMatrix();
	const Vector3 grip(
		(hand.m[3][0] + middle.m[3][0]) * 0.5f,
		(hand.m[3][1] + middle.m[3][1]) * 0.5f,
		(hand.m[3][2] + middle.m[3][2]) * 0.5f);
	m_gunModel.Update(hand, grip);
}

void Player::OnBodyUpdated()
{
	UpdateWireEnds();
	UpdateGunModel();
}

void Player::Render(RenderContext& rc)
{
	Character::Render(rc);
	m_gunModel.Draw(rc);
}

void Player::ResetJumpCount()
{
	m_jumpCount = 0;
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
	Vector3 target = moveDir * m_speed.GetSpeed();

	float t = m_param.move.airControl * g_gameTime->GetFrameDeltaTime();
	if (t > 1.0f)
	{
		t = 1.0f;
	}

	//今の速さを目標の速さに t の割合だけ近づける
	m_moveSpeed.x += (target.x - m_moveSpeed.x) * t;
	m_moveSpeed.z += (target.z - m_moveSpeed.z) * t;

	FaceDirection(moveDir);
}

Vector3 Player::GetMoveDirection() const
{
	return m_command.moveDir;
}

bool Player::FindWall()
{
	Vector3 right = Vector3::Right;
	m_rotation.Apply(right);

	Vector3 from = GetPosition();
	from.y += m_param.body.chestHeight;

	const Vector3 dirs[] = { right, right * -1.0f };
	for (const Vector3& dir : dirs)
	{
		Vector3 to = from + dir * m_param.wallRun.checkLength;
		RayHitInfo hit = nsCollision::RayCast(from, to);

		if (!hit.isHit)
		{
			continue;
		}

		if (fabsf(hit.normal.y) >= m_param.wallRun.normalYLimit)
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

	return IsMoveInput() && speed >= m_param.wallRun.minSpeed && m_reattachTimer <= 0.0f;
}

void Player::StartWallRun()
{
	m_wallRunTimer = 0.0f;
	SetVerticalSpeed(0.0f);

	m_gravity.SetScale(m_param.wallRun.gravityScale);
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

	// 壁走りは最低でも wallRun.speed。速度システムの値がそれより速ければ、その速さで走る
	const float runSpeed = max(m_param.wallRun.speed, m_speed.GetSpeed());
	Vector3 horizontalSpeed = along * runSpeed + m_wallNormal * -m_param.wallRun.stickSpeed;

	m_moveSpeed.x = horizontalSpeed.x;
	m_moveSpeed.z = horizontalSpeed.z;

	FaceDirection(along);
	m_wallRunTimer += g_gameTime->GetFrameDeltaTime();
	if (!IsWallRunTimeOver())
	{
		SetVerticalSpeed(0.0f);
	}
	else
	{
		ApplyGravity();
	}
}

void Player::WallJump()
{
	AddMoveSpeed(m_wallNormal * m_param.wallRun.jumpSpeedOut);
	SetVerticalSpeed(m_param.wallRun.jumpSpeedUp);
	m_jumpCount += 1;
}

void Player::EndWallRun()
{
	m_gravity.SetScale(1.0f);
	m_reattachTimer = m_param.wallRun.reattachTime;
}

bool Player::IsWallRunTimeOver() const
{
	return m_wallRunTimer >= m_param.wallRun.maxTime;
}

void Player::Decelerate()
{
	ApproachHorizontal(Vector3::Zero, m_param.move.groundDecel);
}

bool Player::IsGrappleTrigger() const
{
	return m_command.grappleTrigger;
}

bool Player::IsGrappleRelease() const
{
	return m_command.grappleRelease;
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

	Vector3 toPlayer = GetChestPosition() - m_swingAnchor->GetPosition();
	float distance = toPlayer.Length();

	if (distance < SWING_MIN_DISTANCE)
	{
		return;
	}

	Vector3 toPlayerDir = toPlayer;
	toPlayerDir.Normalize();

	SwingPump(toPlayerDir);

	// ロープの外へ逃げる速さを消す（これで円を描く）
	float outSpeed = m_moveSpeed.Dot(toPlayerDir);
	if (distance >= m_ropeLength && outSpeed > 0.0f)
	{
		m_moveSpeed -= toPlayerDir * outSpeed;
	}

	// 巻き上げ: 最短の長さに近いほどゆっくり引き寄せる（着いた勢いで飛び出さないように）
	float reelSpeed = 0.0f;
	if (m_ropeLength > m_param.swing.minRopeLength)
	{
		reelSpeed = min(m_param.swing.reelSpeed, (distance - m_param.swing.minRopeLength) * m_param.swing.reelEase);

		// 近づいた分だけロープも縮める（伸びることはない）
		m_ropeLength = max(min(m_ropeLength, distance), m_param.swing.minRopeLength);
	}

	// ロープが伸びた分を戻すのに必要な、内向きの速さ
	float pullSpeed = max(distance - m_ropeLength, 0.0f) * m_param.swing.pullRate;

	// 内向きの速さが足りないときだけ足す（毎フレーム足し続けると、ばねのように跳ね飛ばされる）
	// 横の速さ（振り子の勢い）はそのまま残すので、引き寄せられながら振れる
	float needSpeed = max(reelSpeed, pullSpeed);
	float inSpeed = -m_moveSpeed.Dot(toPlayerDir);
	if (inSpeed < needSpeed)
	{
		m_moveSpeed -= toPlayerDir * (needSpeed - inSpeed);
	}

	ClampSwingSpeed();

	Vector3 horizontal = m_moveSpeed;
	horizontal.y = 0.0f;

	if (horizontal.LengthSq() > m_param.swing.faceMinSpeed * m_param.swing.faceMinSpeed)
	{
		FaceDirection(horizontal);
	}
}

void Player::EndSwing()
{
	m_swingAnchor = nullptr;

	// 離したときの速さを上乗せに残す（空中で減速しないように）
	if (m_isHooked)
	{
		Vector3 horizontal = m_moveSpeed;
		horizontal.y = 0.0f;
		m_speed.RaiseBonusTo(horizontal.Length() - m_speed.GetBaseSpeed());
	}

	m_isHooked = false;
	m_grappleWire->SetVisible(false);
}

void Player::ShootWire()
{
	Vector3 origin = GetChestPosition();
	Vector3 toAnchor = m_swingAnchor->GetPosition() - origin;
	float distance = toAnchor.Length();

	m_wireTipLength += m_param.swing.wireShootSpeed * g_gameTime->GetFrameDeltaTime();

	//届いた
	if (m_wireTipLength >= distance)
	{
		HookAnchor(distance);
		return;
	}

	// ワイヤーの見た目は、骨が動いた後の OnBodyUpdated で手から描く
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
		SetVerticalSpeed(m_param.swing.startLift);
	}
}


void Player::ReleaseBoost()
{
	if (!IsGrappleHooked())
	{
		return;
	}

	if (m_moveSpeed.LengthSq() < m_param.swing.releaseMinSpeed * m_param.swing.releaseMinSpeed)
	{
		return;
	}

	Vector3 speed = m_moveSpeed;
	speed.Normalize();
	AddMoveSpeed(speed * m_param.swing.releaseBoost);
	AddMoveSpeed(Vector3::Up * m_param.swing.releaseLift);
}

bool Player::IsShootPress() const
{
	return m_command.shootPress;
}

bool Player::IsAimPress() const
{
	return m_command.aimPress;
}

void Player::UpdateAction()
{
	if (m_controller != nullptr)
	{
		m_controller->MakeCommand(m_command);
	}

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
	UpdateBodyTilt();
	UpdateAnimation();
	UpdateSpeedBonus();
	// 銃口は体の向きから決めるので、先に体を向けてから撃つ
	FaceAimDirection();
	m_gun.Update(IsShootPress(), m_speed.GetLevel(), GetChestPosition(), m_rotation);
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
				score += m_param.lock.keepBonus;
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
	if (distance > m_param.lock.maxDistance)
	{
		return -1.0f;
	}

	Vector3 camToAnchor = anchor->GetPosition() - m_command.aimOrigin;
	camToAnchor.Normalize();
	float dot = m_command.aimDirection.Dot(camToAnchor);
	if (dot < cosf(Math::DegToRad(m_param.lock.maxAngle)))
	{
		return -1.0f;
	}

	float angle = Math::RadToDeg(acosf(min(dot, 1.0f)));

	Vector3 from = GetPosition();
	from.y += m_param.body.chestHeight;		// 胸の高さから飛ばす（足元だと床に当たる）
	if (nsCollision::RayCast(from, anchor->GetPosition()).isHit)
	{
		return -1.0f;
	}

	float angleScore = 1.0f - angle / m_param.lock.maxAngle;
	float distanceScore = 1.0f - distance / m_param.lock.maxDistance;

	return angleScore * m_param.lock.angleWeight + distanceScore * m_param.lock.distanceWeight;
}

Vector3 Player::GetChestPosition() const
{
	Vector3 position = GetPosition();
	position.y += m_param.body.chestHeight;

	return position;
}

void Player::SwingPump(const Vector3& ropeDir)
{
	if (!IsMoveInput())
	{
		return;
	}

	Vector3 input = GetMoveDirection();
	input -= ropeDir * input.Dot(ropeDir);

	if (input.LengthSq() < MOVE_INPUT_THRESHOLD)
	{
		return;
	}

	input.Normalize();
	m_moveSpeed += input * m_param.swing.pumpAccel * g_gameTime->GetFrameDeltaTime();
}

void Player::ClampSwingSpeed()
{
	if (m_moveSpeed.LengthSq() > m_param.swing.maxSpeed * m_param.swing.maxSpeed)
	{
		m_moveSpeed.Normalize();
		m_moveSpeed *= m_param.swing.maxSpeed;
	}
}

void Player::FaceAimDirection()
{
	// 押した最初のフレームは、まだ Gun の構えのタイマーが入っていないので、ボタンも見る
	bool isAiming = IsShootPress() || m_gun.IsAiming() || IsAimPress();
	if (!isAiming || m_stateMachine.IsCurrentState(EnPlayerState::WallRun))
	{
		return;
	}

	FaceDirection(m_command.aimDirection);
}

void Player::UpdateSpeedBonus()
{
	switch (m_stateMachine.GetCurrentState())
	{
	case EnPlayerState::WallRun:
		m_speed.AddBonusPerSecond(m_speedParam.wallRunGain);
		break;
	case EnPlayerState::Run:
		m_speed.AddBonusPerSecond(-m_speedParam.groundDecay);
		break;
	case EnPlayerState::Idle:
		m_speed.AddBonusPerSecond(-m_speedParam.idleDecay);
		break;
	default:
		break;
	}
}

void Player::Warp(const Vector3& position)
{
	// 状態より先に速さを消す（スイング中だと、状態を変えたときに EndSwing が今の速さを上乗せに残してしまうため）
	SetMoveSpeed(Vector3::Zero);
	ChangeState(EnPlayerState::Air);
	m_charaCon.SetPosition(position);
}
