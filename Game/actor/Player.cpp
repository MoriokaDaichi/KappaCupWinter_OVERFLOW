#include "stdafx.h"
#include "actor/Player.h"

#include "controller/IController.h"
#include "actor/playerState/PlayerIdleState.h"
#include "actor/playerState/PlayerRunState.h"
#include "actor/playerState/PlayerAirState.h"
#include "actor/playerState/PlayerWallRunState.h"
#include "actor/playerState/PlayerSwingState.h"
#include "weapon/BulletManager.h"

Player::Player() : m_stateMachine(this)
{
}

bool Player::Start()
{
	LoadAnimation();
	InitCharacter("Assets/modelData/player/medea.tkm", m_animClips.data(), static_cast<int>(m_animClips.size()),
		m_param.body.radius, m_param.body.height, Vector3(0.0f, START_HEIGHT, 0.0f));
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

	m_grapple.Init();

	m_gun.Init(FindGO<BulletManager>("bulletManager"));
	m_gunModel.Init();

	return true;
}

void Player::Move()
{
	const Vector3 moveDir = GetMoveDirection();

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

void Player::ChangeState(const EnPlayerState id)
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

void Player::PlayStateAnimation(const EnPlayerState state)
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

void Player::PlayAnimation(const EnPlayerAnim anim, const bool isRestart)
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
	// 入るとき・抜けるときに急に傾かないように、少しずつ近づける
	const float rate = min(m_param.body.tiltLerpRate * g_gameTime->GetFrameDeltaTime(), 1.0f);
	m_modelTilt.Slerp(rate, m_modelTilt, m_modelTiltTarget);
	m_modelTiltTarget = Quaternion::Identity;
}

void Player::UpdateWireHand()
{
	m_grapple.SetHandPosition(GetRopeHandPosition());
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
	UpdateWireHand();
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

void Player::AddJumpCount()
{
	m_jumpCount += 1;
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

	const Vector3 moveDir = GetMoveDirection();
	const Vector3 target = moveDir * m_speed.GetSpeed();

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

bool Player::CanStartWallRun() const
{
	Vector3 horizontal = GetMoveSpeed();
	horizontal.y = 0.0f;
	const float speed = horizontal.Length();

	return IsMoveInput() && speed >= m_param.wallRun.minSpeed && IsCooldownOver(EnCooldown::WallRun);
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

bool Player::IsShootPress() const
{
	return m_command.shootPress;
}

bool Player::IsAimPress() const
{
	return m_command.aimPress;
}

void Player::StartCooldown(const EnCooldown id, const float seconds)
{
	m_cooldownTimers[static_cast<int>(id)] = seconds;
}

bool Player::IsCooldownOver(const EnCooldown id) const
{
	return m_cooldownTimers[static_cast<int>(id)] <= 0.0f;
}

void Player::UpdateAction()
{
	if (m_controller != nullptr)
	{
		m_controller->MakeCommand(m_command);
	}

	UpdateCooldown();
	m_grapple.UpdateLock(GetPosition(), GetChestPosition(), m_command.aimOrigin, m_command.aimDirection);

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

Vector3 Player::GetChestPosition() const
{
	Vector3 position = GetPosition();
	position.y += m_param.body.chestHeight;

	return position;
}

void Player::FaceAimDirection()
{
	// 押した最初のフレームは、まだ Gun の構えのタイマーが入っていないので、ボタンも見る
	const bool isAiming = IsShootPress() || m_gun.IsAiming() || IsAimPress();
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
	// 状態より先に速さを消す（スイング中だと、状態を変えたときにスイングの State の Exit が今の速さを上乗せに残してしまうため）
	SetMoveSpeed(Vector3::Zero);
	ChangeState(EnPlayerState::Air);
	m_charaCon.SetPosition(position);
}

void Player::UpdateCooldown()
{
	for (float& timer : m_cooldownTimers)
	{
		timer -= g_gameTime->GetFrameDeltaTime();
		if (timer < 0.0f)
		{
			timer = 0.0f;
		}
	}
}
