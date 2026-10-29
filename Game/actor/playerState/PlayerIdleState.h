#pragma once
#include "state/IState.h"

class BattleCharacter;

/** 止まっている状態。入力や接地を見て Run / Air に切り替える */
class PlayerIdleState : public IState<BattleCharacter>
{
public:
	explicit PlayerIdleState(BattleCharacter* owner) : IState<BattleCharacter>(owner){}

	/** ジャンプの回数を戻し、その場で止まる */
	void Enter() override;
	/** 入力と接地を見て、Run / Air に切り替える */
	void Update() override;
};
