#pragma once
#include "actor/characterState/CharacterState.h"

class BattleCharacter;

/** 止まっている状態 */
class IdleState : public CharacterState
{
public:
	explicit IdleState(BattleCharacter* owner) : CharacterState(owner){}

	/** ジャンプの回数を戻し、その場で止まる */
	void Enter() override;
	/** 少しずつ止まる（切り替えは切り替え表が先に見る） */
	void Update() override;
};
