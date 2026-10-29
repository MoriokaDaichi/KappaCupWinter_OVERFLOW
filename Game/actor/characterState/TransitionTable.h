#pragma once
#include "actor/characterState/StateTransition.h"
#include "param/GameParam.h"
#include <vector>

class BattleCharacter;
class CharacterState;

/**
 * 切り替え表。「今の State のとき、条件が成り立ったら、動作をして、次の State へ」を上から順に調べる
 * 表の中身はキャラの JSON の transitions。条件と動作の中身（C++）もここにまとめる
 * BattleCharacter が 1 つ持ち、毎フレーム State を動かす前に呼ぶ
 */
class TransitionTable
{
public:
	/** JSON の表を、名前から enum に直して持つ（毎フレーム文字列を比べないように）。名前が正しいことは起動時に GameParam が確かめている */
	void Init(const std::vector<CharacterParam::Transition>& rows);
	/**
	 * どの State からでも行ける行（from が "Any"）を調べ、成り立った最初の行で切り替える
	 * 例: グラップルを押したら Swing。成り立った行があれば true
	 */
	bool ApplyFromAny(BattleCharacter& owner) const;
	/** 今の State の行を上から調べ、成り立った最初の行で切り替える。成り立った行があれば true */
	bool ApplyFromCurrent(BattleCharacter& owner, const EnCharacterState current, CharacterState& currentState) const;

private:
	/** 1 行の条件が全部成り立っていたら、動作をして切り替える。切り替えたら true */
	bool TryTransition(BattleCharacter& owner, CharacterState* currentState, const StateTransition& transition) const;
	/** 条件 1 つが成り立っているか（State だけが知っている条件は、その State に聞く） */
	bool IsConditionMet(const BattleCharacter& owner, CharacterState* currentState, const EnCondition when) const;
	/** 切り替える前の動作を 1 つする（State だけができる動作は、その State にやってもらう） */
	void DoAction(BattleCharacter& owner, CharacterState* currentState, const EnTransitionAction action) const;

private:
	std::vector<StateTransition> m_rows;		///< 表の行（JSON に書いた順）
};
