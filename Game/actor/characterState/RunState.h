#pragma once
#include "actor/characterState/BattleCharacterState.h"

class BattleCharacter;

/** 走っている状態 */
class RunState : public BattleCharacterState
{
public:
	explicit RunState(BattleCharacter* owner) : BattleCharacterState(owner) {}

	/** ジャンプの回数を戻す */
	void Enter() override;
	/** スティックの向きに走る（切り替えは切り替え表が先に見る） */
	void Update() override;
};
