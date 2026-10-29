#include "stdafx.h"
#include "actor/characterState/TransitionTable.h"

#include "actor/BattleCharacter.h"
#include "actor/characterState/CharacterState.h"
#include "actor/characterState/WallRunState.h"

void TransitionTable::Init(const std::vector<CharacterParam::Transition>& rows)
{
	m_rows.clear();
	for (const CharacterParam::Transition& row : rows)
	{
		StateTransition transition;
		transition.isFromAny = row.from == ANY_STATE_NAME;
		if (!transition.isFromAny)
		{
			ToCharacterState(row.from, transition.from);
		}
		ToCharacterState(row.to, transition.to);

		for (const std::string& name : row.when)
		{
			EnCondition condition = EnCondition::Num;
			ToCondition(name, condition);
			transition.when.push_back(condition);
		}
		for (const std::string& name : row.actions)
		{
			EnTransitionAction action = EnTransitionAction::Num;
			ToTransitionAction(name, action);
			transition.actions.push_back(action);
		}

		m_rows.push_back(transition);
	}
}

bool TransitionTable::ApplyFromAny(BattleCharacter& owner) const
{
	for (const StateTransition& transition : m_rows)
	{
		// "Any" の行には、State だけが知っている条件・動作は書けない（起動時に GameParam が止める）ので、State は渡さない
		if (transition.isFromAny && TryTransition(owner, nullptr, transition))
		{
			return true;
		}
	}
	return false;
}

bool TransitionTable::ApplyFromCurrent(BattleCharacter& owner, const EnCharacterState current, CharacterState& currentState) const
{
	for (const StateTransition& transition : m_rows)
	{
		if (!transition.isFromAny && transition.from == current && TryTransition(owner, &currentState, transition))
		{
			return true;
		}
	}
	return false;
}

bool TransitionTable::TryTransition(BattleCharacter& owner, CharacterState* currentState, const StateTransition& transition) const
{
	// 左から順に調べ、成り立たないものがあればそこでやめる（&& と同じ。重い判定（壁を探すレイ）を後ろに書けば、むだに調べない）
	for (const EnCondition when : transition.when)
	{
		if (!IsConditionMet(owner, currentState, when))
		{
			return false;
		}
	}

	// 動作は、まだ今の State のうちにする（壁ジャンプは WallRun の壁の向きを使うため）。その後で切り替える（今の State の Exit → 次の State の Enter）
	for (const EnTransitionAction action : transition.actions)
	{
		DoAction(owner, currentState, action);
	}
	owner.ChangeState(transition.to);
	return true;
}

bool TransitionTable::IsConditionMet(const BattleCharacter& owner, CharacterState* currentState, const EnCondition when) const
{
	// その State だけが知っている条件（Hooked など）は、今の State に聞く
	if (CONDITION_ONLY_FROM[static_cast<int>(when)] != EnCharacterState::Num)
	{
		return currentState != nullptr && currentState->IsConditionMet(when);
	}

	Vector3 wallNormal = Vector3::Zero;
	switch (when)
	{
	case EnCondition::NotOnGround:
		return !owner.IsOnGround();
	case EnCondition::OnGround:
		return owner.IsOnGround();
	case EnCondition::Landed:
		// 跳んだ直後はまだ接地扱いなので、下に向かっているときだけ着地とみなす
		return owner.IsOnGround() && owner.IsFalling();
	case EnCondition::MoveInput:
		return owner.IsMoveInput();
	case EnCondition::NoMoveInput:
		return !owner.IsMoveInput();
	case EnCondition::JumpTrigger:
		return owner.IsJumpTrigger();
	case EnCondition::JumpStart:
		return owner.IsJumpTrigger() && owner.CanJump();
	case EnCondition::GrappleStart:
		return owner.IsGrappleTrigger() && owner.CanStartSwing();
	case EnCondition::GrappleRelease:
		return owner.IsGrappleRelease();
	case EnCondition::WallFound:
		// 速さを先に見る（足りなければ、レイを飛ばさずに済む）
		return owner.CanStartWallRun() && WallRunState::FindWall(owner, wallNormal);
	case EnCondition::WallLost:
		return !WallRunState::FindWall(owner, wallNormal);
	case EnCondition::Hooked:		// State に聞く条件（上で返している）
	case EnCondition::Num:			// 条件ではない（個数）
		break;
	}
	return false;
}

void TransitionTable::DoAction(BattleCharacter& owner, CharacterState* currentState, const EnTransitionAction action) const
{
	// その State だけができる動作（WallJump など）は、今の State にやってもらう
	if (ACTION_ONLY_FROM[static_cast<int>(action)] != EnCharacterState::Num)
	{
		if (currentState != nullptr)
		{
			currentState->DoAction(action);
		}
		return;
	}

	switch (action)
	{
	case EnTransitionAction::Jump:
		owner.Jump();
		break;
	case EnTransitionAction::WallJump:		// State にやってもらう動作（上でやっている）
	case EnTransitionAction::ReleaseBoost:
	case EnTransitionAction::Num:			// 動作ではない（個数）
		break;
	}
}
