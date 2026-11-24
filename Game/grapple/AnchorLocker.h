#pragma once
#include "param/GameParam.h"

class Anchor;

/** グラップルで狙うアンカーを選ぶ。画面の中央に近いほど、近いほど点が高い */
class AnchorLocker
{
public:
	/** 調整用の値（ロックの分）を受け取る。持ち主の設定のものを借りる */
	explicit AnchorLocker(const CharacterParam::Lock& param) : m_param(param) {}

	/**
	 * 毎フレーム、一番点の高いアンカーを選んでロックする
	 * position: キャラの足元（距離を測る）
	 * chestPosition: 胸の位置（壁にさえぎられていないかを調べる始点）
	 * aimOrigin・aimDirection: 狙いの始点と向き（カメラの位置と前向き）
	 */
	void Update(const Vector3& position, const Vector3& chestPosition,const Vector3& aimOrigin, const Vector3& aimDirection);
	/**
	 * スイングに使うロック中のアンカーを返す。消えていたら nullptr
	 * 中身は .cpp に書く（GOHandle の Get は Anchor の中身を知らないとコンパイルできない。ここは class Anchor; だけなので）
	 */
	Anchor* GetLockedAnchor() const;

private:
	/** アンカー1つの点数を出す。候補にならないときはマイナス（例：-1.0f）を返す */
	float CalcLockScore(const Vector3& position, const Vector3& chestPosition, const Anchor* anchor, const Vector3& aimOrigin, const Vector3& aimDirection) const;

private:
	GOHandle<Anchor> m_lockedAnchor;		///< ロック中のアンカー（借りているだけ）
	const CharacterParam::Lock& m_param;	///< 調整用の値のうち、ロックの分（持ち主の設定から受け取る）
};
