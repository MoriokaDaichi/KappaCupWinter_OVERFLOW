#include "stdafx.h"
#include "actor/BattleCharacter.h"

#include "controller/IController.h"
#include "weapon/BulletManager.h"
#include "actor/characterState/IdleState.h"
#include "actor/characterState/RunState.h"
#include "actor/characterState/AirState.h"
#include "actor/characterState/WallRunState.h"
#include "actor/characterState/SwingState.h"
#include <algorithm>

BattleCharacter::BattleCharacter() : m_stateMachine(this)
{
}

bool BattleCharacter::Start()
{
	// アニメーションは ModelRender の Init に渡すので、体より先に読む
	if (m_param->hasAnimation)
	{
		m_animation = std::make_unique<CharacterAnimation>();
		m_animation->Load(m_param->animation);
	}
	AnimationClip* clips = m_animation ? m_animation->GetClips() : nullptr;
	const int clipCount = m_animation ? m_animation->GetClipCount() : 0;
	InitCharacter(m_param->model.c_str(), clips, clipCount, m_param->body.radius, m_param->body.height, m_startPosition);

	if (m_animation)
	{
		m_animation->SetModel(m_modelRender);
	}

	if (m_param->hasHands)
	{
		m_handBones = std::make_unique<HandBones>();
		m_handBones->Init(m_modelRender, m_param->hands);
	}

	// 最初の状態に切り替えるとアニメーションも切り替わるので、アニメーションの準備の後に呼ぶ
	InitState();

	InitAbilities();

	// 銃の見た目は、銃を持っていて、握る手の骨がわかるキャラだけ
	if (HasGun() && m_handBones)
	{
		m_gunModel = std::make_unique<GunModel>();
		m_gunModel->Init();
	}

	return true;
}

void BattleCharacter::Warp(const Vector3& position)
{
	// 状態より先に速さを消す（スイング中だと、状態を変えたときにスイングの State の Exit が今の速さを上乗せに残してしまうため）
	SetMoveSpeed(Vector3::Zero);
	ChangeState(EnCharacterState::Air);
	m_charaCon.SetPosition(position);
}

bool BattleCharacter::IsJumpTrigger() const
{
	return m_command.jumpTrigger;
}

bool BattleCharacter::IsMoveInput() const
{
	return m_command.moveDir.LengthSq() > MOVE_INPUT_THRESHOLD;
}

Vector3 BattleCharacter::GetMoveDirection() const
{
	return m_command.moveDir;
}

bool BattleCharacter::IsGrappleTrigger() const
{
	return m_command.grappleTrigger;
}

bool BattleCharacter::IsGrappleRelease() const
{
	return m_command.grappleRelease;
}

bool BattleCharacter::IsShootPress() const
{
	return m_command.shootPress;
}

bool BattleCharacter::IsAimPress() const
{
	return m_command.aimPress;
}

void BattleCharacter::StartCooldown(const EnCooldown id, const float seconds)
{
	m_cooldownTimers[static_cast<int>(id)] = seconds;
}

bool BattleCharacter::IsCooldownOver(const EnCooldown id) const
{
	return m_cooldownTimers[static_cast<int>(id)] <= 0.0f;
}

void BattleCharacter::Move()
{
	const Vector3 moveDir = GetMoveDirection();

	//速度に入れる
	ApproachHorizontal(moveDir * GetSpeedSystem().GetSpeed(), m_param->move.groundAccel);

	//スティックを倒しているときだけ向きを変える（離したときは今の向きを保つ）
	FaceDirection(moveDir);
}

void BattleCharacter::AirMove()
{
	if (!IsMoveInput())
	{
		return;
	}

	const Vector3 moveDir = GetMoveDirection();
	const Vector3 target = moveDir * GetSpeedSystem().GetSpeed();

	float t = m_param->move.airControl * g_gameTime->GetFrameDeltaTime();
	if (t > 1.0f)
	{
		t = 1.0f;
	}

	//今の速さを目標の速さに t の割合だけ近づける
	m_moveSpeed.x += (target.x - m_moveSpeed.x) * t;
	m_moveSpeed.z += (target.z - m_moveSpeed.z) * t;

	FaceDirection(moveDir);
}

void BattleCharacter::Decelerate()
{
	ApproachHorizontal(Vector3::Zero, m_param->move.groundDecel);
}

void BattleCharacter::Jump()
{
	// = で入れるので、落ちている途中に跳んでも同じ高さだけ上がる
	SetVerticalSpeed(m_param->jump.speed);
	m_jumpCount += 1;
	// 2段目のジャンプでも、踏み切りのアニメーションから流し直す
	if (m_animation)
	{
		m_animation->OnJump();
	}
}

void BattleCharacter::ResetJumpCount()
{
	m_jumpCount = 0;
}

void BattleCharacter::AddJumpCount()
{
	m_jumpCount += 1;
}

void BattleCharacter::ConsumeGroundJump()
{
	if (m_jumpCount == 0)
	{
		m_jumpCount = 1;
	}
}

bool BattleCharacter::CanStartWallRun() const
{
	Vector3 horizontal = GetMoveSpeed();
	horizontal.y = 0.0f;
	const float speed = horizontal.Length();

	return IsMoveInput() && speed >= m_param->wallRun.minSpeed && IsCooldownOver(EnCooldown::WallRun);
}

Vector3 BattleCharacter::GetChestPosition() const
{
	Vector3 position = GetPosition();
	position.y += m_param->body.chestHeight;

	return position;
}

void BattleCharacter::ChangeState(const EnCharacterState id)
{
	m_stateMachine.ChangeState(id);
	if (m_animation)
	{
		m_animation->OnStateChanged(id, GetMoveSpeed().y);
	}
}

void BattleCharacter::InitState()
{
	for (const std::string& name : m_param->states)
	{
		EnCharacterState id = EnCharacterState::Num;
		const bool found = ToCharacterState(name, id);

		K2_ASSERT(found, "知らない State の名前です");

		AddStateById(id);
	}

	m_transitionTable.Init(m_param->transitions);

	ChangeState(EnCharacterState::Idle);
}

void BattleCharacter::UpdateState()
{
	// 1. どの State からでも行ける行（例: グラップルを押したら Swing）。
	//    ここで切り替えても、このフレームはそのまま新しい State の行を調べて、動かす（スイングは押したフレームからワイヤーを飛ばすため）
	m_transitionTable.ApplyFromAny(*this);

	// 2. 今の State の行。切り替えたら、このフレームは動かさない（新しい State は次のフレームから動く）
	if (m_transitionTable.ApplyFromCurrent(*this, GetCurrentState(), *m_stateMachine.GetCurrentStateObject()))
	{
		return;
	}

	// 3. どの行も成り立たなかったので、今の State の動きをする
	m_stateMachine.Update();
}

void BattleCharacter::UpdateAction()
{
	UpdateCommand();
	if (HasGrapple())
	{
		GetGrapple().UpdateLock(GetPosition(), GetChestPosition(), GetCommand().aimOrigin, GetCommand().aimDirection);
	}

	UpdateState();
	// State が決めた傾きとアニメーションを、State を動かした直後に合わせる
	UpdateModelTilt();
	if (m_animation)
	{
		m_animation->Update(GetCurrentState(), GetMoveSpeed().y);
	}
	UpdateSpeedBonus();
	// 銃口は体の向きから決めるので、先に体を向けてから撃つ
	FaceAimDirection();
	if (HasGun())
	{
		GetGun().Update(IsShootPress(), IsAimPress(), GetSpeedSystem().GetLevel(), GetChestPosition(), m_rotation, GetCommand().aimOrigin, GetCommand().aimDirection);
	}
}

void BattleCharacter::UpdateCommand()
{
	if (m_controller != nullptr)
	{
		m_controller->MakeCommand(m_command);
	}

	UpdateCooldown();
}

void BattleCharacter::UpdateCooldown()
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

void BattleCharacter::UpdateModelTilt()
{
	// 入るとき・抜けるときに急に傾かないように、少しずつ近づける
	const float rate = min(m_param->body.tiltLerpRate * g_gameTime->GetFrameDeltaTime(), 1.0f);
	m_modelTilt.Slerp(rate, m_modelTilt, m_modelTiltTarget);
	// 目標は State が毎フレーム出し直す。出さなかったフレームはまっすぐに戻す
	m_modelTiltTarget = Quaternion::Identity;
}

void BattleCharacter::OnBodyUpdated()
{
	// ワイヤーの手元は両手の間。手の骨がわからないキャラは胸から出す
	if (HasGrapple())
	{
		Vector3 hand = GetChestPosition();
		if (m_handBones)
		{
			m_handBones->GetRopeHandPosition(hand);
		}
		GetGrapple().SetHandPosition(hand);
	}

	// 銃の見た目を右手に持たせる
	if (m_gunModel)
	{
		Matrix handMatrix;
		Vector3 gripPosition;
		if (m_handBones->GetGrip(handMatrix, gripPosition))
		{
			m_gunModel->Update(handMatrix, gripPosition);
		}
	}
}

void BattleCharacter::Render(RenderContext& rc)
{
	Character::Render(rc);
	if (m_gunModel)
	{
		m_gunModel->Draw(rc);
	}
}

void BattleCharacter::FaceAimDirection()
{
	// 押した最初のフレームは、まだ Gun の構えのタイマーが入っていないので、ボタンも見る
	const bool isAiming = IsShootPress() || (HasGun() && GetGun().IsAiming()) || IsAimPress();
	if (!isAiming || GetCurrentState() == EnCharacterState::WallRun)
	{
		return;
	}

	FaceDirection(GetCommand().aimDirection);
}

void BattleCharacter::UpdateSpeedBonus()
{
	switch (GetCurrentState())
	{
	case EnCharacterState::WallRun:
		GetSpeedSystem().AddBonusPerSecond(m_speedParam.wallRunGain);
		break;
	case EnCharacterState::Run:
		GetSpeedSystem().AddBonusPerSecond(-m_speedParam.groundDecay);
		break;
	case EnCharacterState::Idle:
		GetSpeedSystem().AddBonusPerSecond(-m_speedParam.idleDecay);
		break;
	default:
		break;
	}
}

bool BattleCharacter::HasAbility(const char* name) const
{
	const std::vector<std::string>& abilities = m_param->abilities;
	return std::find(abilities.begin(), abilities.end(), name) != abilities.end();
}

void BattleCharacter::AddStateById(const EnCharacterState id)
{
	switch (id)
	{
	case EnCharacterState::Idle:
		m_stateMachine.AddState<IdleState>(id);
		break;
	case EnCharacterState::Run:
		m_stateMachine.AddState<RunState>(id);
		break;
	case EnCharacterState::Air:
		m_stateMachine.AddState<AirState>(id);
		break;
	case EnCharacterState::WallRun:
		m_stateMachine.AddState<WallRunState>(id);
		break;
	case EnCharacterState::Swing:
		m_stateMachine.AddState<SwingState>(id);
		break;
	case EnCharacterState::Num:		// State ではない（個数）ので作らない
		break;
	}
}

void BattleCharacter::InitAbilities()
{
	m_speed.Reset();
	if (HasAbility("Grapple"))
	{
		m_grapple = std::make_unique<Grapple>(m_param->lock);
		m_grapple->Init();
	}
	if (HasAbility("Gun"))
	{
		m_gun = std::make_unique<Gun>();
		m_gun->Init(FindGO<BulletManager>("bulletManager"));
	}
}
