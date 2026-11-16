#pragma once
#include "param/GameParam.h"

class BattleCharacter;

/**
 * 画面の中央の四角に入れ続けた敵を、順にロックしていく（マルチロックオン。ボタンは使わず、いつも動く）
 * 四角から出た敵はロックしかけに戻し、lockTime の間に戻らなければ外す（柱の陰に一瞬入っただけで外れないように）
 * 敵はロック中に倒されて消えることがあるので、ポインタではなく GOHandle で覚える
 */
class MultiLocker
{
public:
	/** ロックしかけている敵 1 体分 */
	struct Candidate
	{
		GOHandle<BattleCharacter> target;	///< 敵
		float progress = 0.0f;				///< ロックまでの進み具合（0〜1。1 でロック）
	};

	/**
	 * 四角に入っている敵の進み具合を増やし、外れた敵は減らす。満ちた敵をロックする（maxCountまで）
	 * aimOrigin: 狙いの始点（カメラの位置。距離と壁の判定に使う）
	 * boxHalfSize: 四角の幅と高さの半分（ピクセル。画面の中央が中心）
	 */
	void Update(const Vector3& aimOrigin, const Vector2& boxHalfSize, const int maxCount);
	/** 倒された敵をロックから外す。毎フレーム呼ぶ（Update を呼ばない Lv でも、ロックしかけの敵が倒されることがある） */
	void RemoveDeadTargets();
	/** ロックした敵を返す（ロックした順）。使う前に Get で生きているかを確かめる */
	const std::vector<GOHandle<BattleCharacter>>& GetTargets() const { return m_targets; }
	/** ロックしかけている敵を返す（HUD でドーナツを出すため） */
	const std::vector<Candidate>& GetCandidates() const { return m_candidates; }
	/** ロックと、ロックしかけている敵を全部外す（撃った後に呼ぶ） */
	void Clear();

private:
	/** target をロックできるか（距離・カメラの前にいるか・四角の中か・壁にさえぎられていないか。軽い判定から順に見る） */
	bool CanLock(const BattleCharacter& target, const Vector3& aimOrigin, const Vector2& boxHalfSize) const;
	/** ロックした敵のうち、四角から出た敵をロックしかけ（進み具合 1）に戻す */
	void ReleaseOutOfBox(const Vector3& aimOrigin, const Vector2& boxHalfSize);
	/** もうロックしているか */
	bool IsLocked(const BattleCharacter* target) const;
	/** ロックしかけている敵の中から target を探す。いなければ nullptr */
	Candidate* FindCandidate(const BattleCharacter* target);

private:
	std::vector<GOHandle<BattleCharacter>> m_targets;		///< ロックした敵
	std::vector<Candidate> m_candidates;					///< ロックしかけている敵
	const GunParam& m_param = nsGameParam::Get().gun;		///< 調整用の値（gun.json）

	static constexpr float MIN_LOCK_TIME = 0.01f;			///< lockTime が 0 のときに 0 で割らないための下限（秒）
};
