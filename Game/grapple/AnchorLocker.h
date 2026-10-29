#pragma once
#include "param/GameParam.h"

class Anchor;

/** グラップルで狙うアンカーを選ぶ。画面の中央に近いほど、近いほど点が高い */
class AnchorLocker
{
public:
	/**
	 * 毎フレーム、一番点の高いアンカーを選んでロックする
	 * position: キャラの足元（距離を測る）
	 * chestPosition: 胸の位置（壁にさえぎられていないかを調べる始点）
	 * aimOrigin・aimDirection: 狙いの始点と向き（カメラの位置と前向き）
	 */
	void Update(const Vector3& position, const Vector3& chestPosition,const Vector3& aimOrigin, const Vector3& aimDirection);
	/** スイングに使うロック中のアンカーを返す */
	Anchor* GetLockedAnchor() const { return m_lockedAnchor; }

private:
	/** アンカー1つの点数を出す。候補にならないときはマイナス（例：-1.0f）を返す */
	float CalcLockScore(const Vector3& position, const Vector3& chestPosition, const Anchor* anchor, const Vector3& aimOrigin, const Vector3& aimDirection) const;

private:
	Anchor* m_lockedAnchor = nullptr;		///< ロック中のアンカー（借りているだけ）
	const CharacterParam::Lock& m_param = nsGameParam::Get().player.lock;		///< 調整用の値のうち、ロックの分（Assets/param/player.json）
};
