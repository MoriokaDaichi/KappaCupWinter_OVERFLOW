#pragma once
#include "controller/IController.h"

class BattleCharacter;

/** 敵用のコマンドを作る。今は「追いかける相手の方へ歩く」だけ（仮: 撃つ・跳ぶなどは敵のタスクで足す） */
class AIController : public IController
{
public:
	/** 自分と、追いかける相手から、このフレームのコマンドを作る */
	void MakeCommand(CharacterCommand& command) const override;
	/** コマンドを作る本人を設定する（位置を見るだけ） */
	void SetOwner(const BattleCharacter* owner) { m_owner = owner; }
	/** 追いかける相手を設定する（位置を見るだけ） */
	void SetTarget(const BattleCharacter* target) { m_target = target; }

private:
	static constexpr float STOP_DISTANCE = 300.0f;		///< 相手にこれより近づいたら止まる（ぶつかり続けないように）

	const BattleCharacter* m_owner = nullptr;		///< コマンドを作る本人（借りているだけ）
	const BattleCharacter* m_target = nullptr;		///< 追いかける相手（借りているだけ）
};
