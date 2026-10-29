#include "stdafx.h"
#include "actor/CharacterAnimation.h"

namespace
{
	/** 状態ごとのアニメーション（EnCharacterState と同じ順に並べる）。壁走り専用はないので run を使う */
	const EnCharacterAnim STATE_TO_ANIM[] = {
		EnCharacterAnim::Idle,		// Idle
		EnCharacterAnim::Run,		// Run
		EnCharacterAnim::Fall,		// Air（実際は SelectAirAnimation で選ぶ）
		EnCharacterAnim::Run,		// WallRun（体を傾けて、壁を走って見せる）
		EnCharacterAnim::Swing,		// Swing
	};
	static_assert(sizeof(STATE_TO_ANIM) / sizeof(STATE_TO_ANIM[0]) == static_cast<int>(EnCharacterState::Num),
		"STATE_TO_ANIM を EnCharacterState に合わせてください");
}

void CharacterAnimation::Load(const CharacterParam::Animation& param)
{
	m_param = &param;

	// 名前とすべてそろっているかは、起動時に GameParam が確かめている
	for (const CharacterParam::Animation::Clip& clip : param.clips)
	{
		EnCharacterAnim anim = EnCharacterAnim::Num;
		const bool found = ToCharacterAnim(clip.anim, anim);
		K2_ASSERT(found, "知らないアニメーションの名前です");

		AnimationClip& animClip = m_clips[static_cast<int>(anim)];
		animClip.Load(clip.file.c_str());
		animClip.SetLoopFlag(clip.loop);
	}
}

void CharacterAnimation::OnJump()
{
	//2段目のジャンプでも、踏み切りから流し直す
	Play(EnCharacterAnim::JumpUp, true);
}

void CharacterAnimation::OnStateChanged(const EnCharacterState state, const float verticalSpeed)
{
	if (state == EnCharacterState::Air)
	{
		Play(SelectAirAnimation(verticalSpeed));
		return;
	}

	// 止まって着地したときだけ、着地の動きを流す（走りながらの着地は、足が止まって見えるので run のまま）
	const bool isFromAir = m_currentAnim == EnCharacterAnim::JumpUp || m_currentAnim == EnCharacterAnim::Fall || m_currentAnim == EnCharacterAnim::FallFast;
	if (state == EnCharacterState::Idle && isFromAir && m_airFallSpeed > m_param->landMinSpeed)
	{
		Play(EnCharacterAnim::Land);
		return;
	}

	Play(STATE_TO_ANIM[static_cast<int>(state)]);
}

void CharacterAnimation::Update(const EnCharacterState state, const float verticalSpeed)
{
	if (state == EnCharacterState::Air)
	{
		m_airFallSpeed = max(-verticalSpeed, 0.0f);
		Play(SelectAirAnimation(verticalSpeed));
		return;
	}

	if (m_currentAnim == EnCharacterAnim::Land && !m_model->IsPlayingAnimation())
	{
		Play(EnCharacterAnim::Idle);
	}
}

EnCharacterAnim CharacterAnimation::SelectAirAnimation(const float verticalSpeed) const
{
	// 踏み切り（Jump で流し始める）は最後まで流す。Jump の後に Air に入るので、ここで消さないようにする
	if (m_currentAnim == EnCharacterAnim::JumpUp && m_model->IsPlayingAnimation())
	{
		return EnCharacterAnim::JumpUp;
	}

	// 高いところから落ちて、速さがついてきた
	if (verticalSpeed < -m_param->fastFallSpeed)
	{
		return EnCharacterAnim::FallFast;
	}

	return EnCharacterAnim::Fall;
}

void CharacterAnimation::Play(const EnCharacterAnim anim, const bool isRestart)
{
	// 壁走り→走るのように同じアニメーションが続くときは、最初から流し直さない
	if (anim == m_currentAnim && !isRestart)
	{
		return;
	}

	m_model->PlayAnimation(static_cast<int>(anim), INTERPOLATE_TIME);
	m_currentAnim = anim;
}
