#pragma once
#include "actor/characterState/BattleCharacterState.h"

class BattleCharacter;

/** 重力なしで、命令の向きへ飛ぶ状態 */
class FlyState : public BattleCharacterState
{
public:
	explicit FlyState(BattleCharacter* owner) : BattleCharacterState(owner) {}
	/** 命令の向き × 飛ぶ速さへ、1 秒に accel ずつ速度を近づける（重力はかけない） */
	void Update() override;
};
