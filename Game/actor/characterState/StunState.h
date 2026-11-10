#pragma once
#include "actor/characterState/BattleCharacterState.h"

class BattleCharacter;

/**
 * しびれて動けない状態（電気柵・レーザーに当たったとき）。BattleCharacter::Stun で入る
 * 入力は BattleCharacter が空にするので、はじかれた勢いのまま飛び、地面では止まっていく
 */
class StunState : public BattleCharacterState
{
public:
	explicit StunState(BattleCharacter* owner) : BattleCharacterState(owner) {}

	/** しびれている時間を数え始める */
	void Enter() override;
	/** 重力と、地面での減速（切り替えは切り替え表が先に見る） */
	void Update() override;
	/** StunOver（しびれている時間が過ぎた）に答える */
	bool IsConditionMet(const EnCondition when) const override;

private:
	static constexpr float STUN_TIME = 0.8f;	///< しびれて動けない秒数

	float m_timer = 0.0f;						///< しびれてからの秒数
};
