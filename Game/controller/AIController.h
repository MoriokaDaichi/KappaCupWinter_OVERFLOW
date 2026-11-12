#pragma once
#include "controller/IController.h"
#include "controller/ShootDecider.h"

class BattleCharacter;

/**
 * 敵用のコマンドを作る。
 * 追いかける相手の方へ歩き、その相手に撃つ（撃つのは shooter を書いた敵だけ）。
 * SetPatrol を呼ぶと追いかけず、始点と終点の間を往復する。
 */
class AIController : public IController
{
public:
	/** 自分の位置と、追いかける相手か往復の端から、このフレームのコマンドを作る */
	void MakeCommand(CharacterCommand& command) override;
	/** コマンドを作る本人を設定する（位置を見るだけ） */
	void SetOwner(const BattleCharacter* owner) { m_owner = owner; }
	/** 追いかける相手を設定する（位置を見るだけ） */
	void SetTarget(const BattleCharacter* target) { m_target = target; }
	/** JSON の shooter の数値を受け取る（借りているだけ。撃つ敵だけ呼ぶ） */
	void InitShooter(const CharacterParam::Shooter& param) { m_shootDecider.Init(param); }
	/** 撃つ段階と往復の向かう先を、最初に戻す（湧き直したとき・最初からやり直すとき） */
	void Reset() { m_shootDecider.Reset(); m_patrolIndex = 1; }
	/** 始点と終点を受け取り、その間を往復させる（往復する敵だけ呼ぶ。呼んだ敵は追いかけなくなる） */
	void SetPatrol(const Vector3& start, const Vector3& end);

private:
	static constexpr float STOP_DISTANCE = 300.0f;		///< 相手にこれより近づいたら止まる（ぶつかり続けないように）
	static constexpr float PATROL_ARRIVE_DISTANCE = 50.0f;	///< 往復の端にこれより近づいたら着いたとして折り返す（ぴったりには止まれないので）

	const BattleCharacter* m_owner = nullptr;		///< コマンドを作る本人（借りているだけ）
	const BattleCharacter* m_target = nullptr;		///< 追いかける相手（借りているだけ）
	ShootDecider m_shootDecider;					///< 敵が撃つかどうかを決める係。InitShooter を呼んだ敵だけ動く
	Vector3 m_patrolPoints[2];						///< 行き来する両端の場所（[0] が始点（湧く場所）、[1] が終点（patrolTo））
	int m_patrolIndex = 1;							///< 今向かっている方の番号（0: 始点、1: 終点）
	bool m_hasPatrol = false;						///< 往復する敵かどうか（SetPatrol を呼んだ敵だけ true）
};
