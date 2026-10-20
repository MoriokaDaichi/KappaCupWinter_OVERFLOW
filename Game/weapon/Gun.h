#pragma once
#include "param/GameParam.h"

/** RT を押している間、一定の間隔で照準の先へ弾を撃つ。Player が持つ */
class Gun
{
public:
	/** 毎フレーム Player が呼ぶ。押していて間隔が空いていれば撃つ。muzzle は弾が出る位置 */
	void Update(bool isShootPress, const Vector3& muzzle);
	/** 構えているか（撃っている間と、やめてから aimHoldTime 秒の間） */
	bool IsAiming() const { return m_aimTimer > 0.0f; }

private:
	/** 照準の先へ向けて弾を1発出す */
	void Shoot(const Vector3& muzzle);
	/** 画面の中央の先にある、狙う点を返す */
	Vector3 CalcAimPoint() const;

private:
	const GunParam& m_param = nsGameParam::Get().gun;		///< 調整用の値（Assets/param/gun.json）
	float m_shootTimer = 0.0f;								///< 次に撃てるまでの残り時間（秒）
	float m_aimTimer = 0.0f;								///< 構えをやめるまでの残り時間（秒）

	static constexpr float AIM_MIN_DISTANCE = 1.0f;			///< 狙う点が手元からこれより近いと、向きを出せない
};
