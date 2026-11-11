#pragma once
#include "controller/IController.h"
#include "controller/ShootDecider.h"

class BattleCharacter;

/** 敵用のコマンドを作る。追いかける相手の方へ歩き、その相手に撃つ（shooter を書いた敵だけ）（仮: 跳ぶなどは敵のタスクで足す） */
class AIController : public IController
{
public:
	/** 自分と、追いかける相手から、このフレームのコマンドを作る */
	void MakeCommand(CharacterCommand& command) override;
	/** コマンドを作る本人を設定する（位置を見るだけ） */
	void SetOwner(const BattleCharacter* owner) { m_owner = owner; }
	/** 追いかける相手を設定する（位置を見るだけ） */
	void SetTarget(const BattleCharacter* target) { m_target = target; }
	/** JSON の shooter の数値を受け取る（借りているだけ。撃つ敵だけ呼ぶ） */
	void InitShooter(const CharacterParam::Shooter& param) { m_shootDecider.Init(param); }
	/** 撃つかどうかを決める係の段階を待機に戻す（湧き直したとき・最初からやり直すとき） */
	void Reset() { m_shootDecider.Reset(); }

private:
	static constexpr float STOP_DISTANCE = 300.0f;		///< 相手にこれより近づいたら止まる（ぶつかり続けないように）

	const BattleCharacter* m_owner = nullptr;		///< コマンドを作る本人（借りているだけ）
	const BattleCharacter* m_target = nullptr;		///< 追いかける相手（借りているだけ）
	ShootDecider m_shootDecider;					///< 敵が撃つかどうかを決める係。InitShooter を呼んだ敵だけ動く
};
