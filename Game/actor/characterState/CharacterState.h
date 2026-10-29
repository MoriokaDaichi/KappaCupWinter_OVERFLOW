#pragma once
#include "state/IState.h"
#include "actor/characterState/StateTransition.h"

class BattleCharacter;

/**
 * キャラクターの State の親。IState に「切り替え表から呼ばれる 2 つの窓口」を足したもの
 * 切り替えの判断は表（BattleCharacter）がするので、State の Update には動きだけを書く
 */
class CharacterState : public IState<BattleCharacter>
{
public:
	explicit CharacterState(BattleCharacter* owner) : IState<BattleCharacter>(owner) {}

	/**
	 * この State だけが判定できる条件に答える（CONDITION_ONLY_FROM でこの State を指している条件だけが来る）
	 * 例: Swing の Hooked（ワイヤーが届いたかは Swing だけが知っている）
	 */
	virtual bool IsConditionMet(const EnCondition when) const { return false; }
	/**
	 * この State だけができる動作をする（ACTION_ONLY_FROM でこの State を指している動作だけが来る）
	 * 例: WallRun の WallJump（壁の向きは WallRun だけが知っている）
	 */
	virtual void DoAction(const EnTransitionAction action) {}
};
