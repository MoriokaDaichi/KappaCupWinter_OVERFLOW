#pragma once
#include "grapple/AnchorLocker.h"

class GrappleWire;
class Anchor;

/** グラップルの能力。狙うアンカーの選択と、ワイヤーの見た目を持つ */
class Grapple : public Noncopyable
{
public:
	/** ロックの値を受け取る（AnchorLocker に渡す） */
	explicit Grapple(const CharacterParam::Lock& lockParam);
	/** ワイヤーを消す */
	~Grapple();
	/** ワイヤーを作る */
	void Init();

public:
	/**
	 * 毎フレーム、一番点の高いアンカーを選んでロックする
	 * position: キャラの足元（距離を測る）
	 * chestPosition: 胸の位置（壁にさえぎられていないかを調べる始点）
	 * aimOrigin・aimDirection: 狙いの始点と向き（カメラの位置と前向き）
	 */
	void UpdateLock(const Vector3& position, const Vector3& chestPosition, const Vector3& aimOrigin, const Vector3& aimDirection);
	/** ロック中のアンカーを返す。候補がなければ nullptr */
	Anchor* GetLockedAnchor() const { return m_locker.GetLockedAnchor(); }
	/** ワイヤーの手元を渡す（骨が動いた後に呼ぶ） */
	void SetHandPosition(const Vector3& hand);
	/** ワイヤーの見た目を返す（スイングの State が使う） */
	GrappleWire& GetWire();

private:
	AnchorLocker m_locker;				///< 最適なアンカーをえらぶ
	GrappleWire* m_wire = nullptr;		///< ワイヤーの見た目（Init で作り、デストラクタで消す）
};
