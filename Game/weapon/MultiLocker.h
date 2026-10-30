#pragma once
#include "param/GameParam.h"

class BattleCharacter;

/** LTを押している間、照準に入れ続けた敵を順にロックしていく（マルチロックオン） */
class MultiLocker
{
public:
	/** ロックしかけている敵 1 体分 */
	struct Candidate
	{
		BattleCharacter* target = nullptr;	///< 敵（借りているだけ）
		float progress = 0.0f;				///< ロックまでの進み具合（0〜1。1 でロック）
	};

	/**
	 * 照準に入っている敵の進み具合を増やし、外れた敵は減らす。満ちた敵をロックする（maxCountまで）
	 * aimOrigin・aimDirection: 狙いの始点と向き（カメラの位置と前向き）
	 */
	void Update(const Vector3& aimOrigin, const Vector3& aimDirection, const int maxCount);
	/** ロックした敵を返す（ロックした順） */
	const std::vector<BattleCharacter*>& GetTargets() const { return m_targets; }
	/** ロックしかけている敵を返す（HUD でドーナツを出すため） */
	const std::vector<Candidate>& GetCandidates() const { return m_candidates; }
	/** ロックと、ロックしかけている敵を全部外す（撃った後に呼ぶ） */
	void Clear();

private:
	/** target をロックできるか（距離・照準からの角度・壁にさえぎられていないか） */
	bool CanLock(const BattleCharacter& target, const Vector3& aimOrigin, const Vector3& aimDirection) const;
	/** もうロックしているか */
	bool IsLocked(const BattleCharacter* target) const;
	/** ロックしかけている敵の中から target を探す。いなければ nullptr */
	Candidate* FindCandidate(const BattleCharacter* target);

private:
	std::vector<BattleCharacter*> m_targets;				///< ロックした敵（借りているだけ）
	std::vector<Candidate> m_candidates;					///< ロックしかけている敵
	const GunParam& m_param = nsGameParam::Get().gun;		///< 調整用の値（gun.json）

	static constexpr float MIN_LOCK_TIME = 0.01f;			///< lockTime が 0 のときに 0 で割らないための下限（秒）
};
