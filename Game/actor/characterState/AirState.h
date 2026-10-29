#pragma once
#include "actor/characterState/CharacterState.h"

class BattleCharacter;

/** 空中にいる状態 */
class AirState : public CharacterState
{
public:
	explicit AirState(BattleCharacter* owner) : CharacterState(owner) {}

	/** 足場から落ちてきたときは、地上のジャンプを使ったことにする */
	void Enter() override;
	/** 2 段目のジャンプ・空中の移動・重力（切り替えは切り替え表が先に見る） */
	void Update() override;
};
