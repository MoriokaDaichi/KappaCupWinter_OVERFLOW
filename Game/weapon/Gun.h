#pragma once
#include "param/GameParam.h"

class BulletManager;

/** RT を押している間、一定の間隔で照準の先へ弾を撃つ。BattleCharacter が持つ（abilities に "Gun" があるときだけ） */
class Gun
{
public:
	/** 弾を出す BulletManager を受け取る（借りているだけ。消すのは Game） */
	void Init(BulletManager* bulletManager) { m_bulletManager = bulletManager; }
	/** 毎フレーム BattleCharacter が呼ぶ。押していて間隔が空いていれば撃つ。level は今の速度 Lv（1〜5）。chest は胸の位置、rotation は体の向き（銃口の位置を決める） */
	void Update(const bool isShootPress, const int level, const Vector3& chest, const Quaternion& rotation);
	/** 構えているか（撃っている間と、やめてから aimHoldTime 秒の間） */
	bool IsAiming() const { return m_aimTimer > 0.0f; }

private:
	/** 照準の先へ向けて弾を1発出す */
	void Shoot(const Vector3& muzzle, const GunParam::Level& levelParam);
	/** 画面の中央の先にある、狙う点を返す */
	Vector3 CalcAimPoint() const;
	/** 銃口の位置を返す（胸から体の右前へずらした点）。壁に埋まるときは胸を返す */
	Vector3 CalcMuzzle(const Vector3& chest, const Quaternion& rotation) const;

private:
	BulletManager* m_bulletManager = nullptr;				///< 弾を出す先（借りているだけ）
	const GunParam& m_param = nsGameParam::Get().gun;		///< 調整用の値（Assets/param/gun.json）
	float m_shootTimer = 0.0f;								///< 次に撃てるまでの残り時間（秒）
	float m_aimTimer = 0.0f;								///< 構えをやめるまでの残り時間（秒）

	static constexpr float AIM_MIN_DISTANCE = 1.0f;			///< 狙う点が手元からこれより近いと、向きを出せない
};
