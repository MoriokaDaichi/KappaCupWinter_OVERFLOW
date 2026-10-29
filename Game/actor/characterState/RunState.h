#pragma once
#include "actor/characterState/CharacterState.h"

class BattleCharacter;

/** 走っている状態 */
class RunState : public CharacterState
{
public:
	explicit RunState(BattleCharacter* owner) : CharacterState(owner) {}

	/** ジャンプの回数を戻す */
	void Enter() override;
	/** スティックの向きに走る（切り替えは切り替え表が先に見る） */
	void Update() override;
};
