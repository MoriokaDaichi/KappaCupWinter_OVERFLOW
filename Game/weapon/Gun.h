#pragma once
#include "param/GameParam.h"
#include "weapon/MultiLocker.h"

class BulletManager;

/**
 * RT を押している間、一定の間隔で照準の先へ弾を撃つ。Lv3 からは LT で敵をロックし、離すとホーミング弾を撃つ。
 * BattleCharacter が持つ（abilities に "Gun" があるときだけ）
 */
class Gun
{
public:
	/** 弾を出す BulletManager を受け取る（借りているだけ。消すのは Game） */
	void Init(BulletManager* bulletManager) { m_bulletManager = bulletManager; }
	/** 
	 * 毎フレーム BattleCharacter が呼ぶ。押していて間隔が空いていれば撃つ。
	 * level は今の速度 Lv（1〜5）。chest は胸の位置、rotation は体の向き（銃口の位置を決める） 
	 * isAimPress は LT（構え）。押している間に敵をロックし、離したらロックした敵へ撃つ
	 * aimOrigin・aimDirection は狙いの始点と向き（マルチロックオンで使う）
	 */
	void Update(const bool isShootPress, const bool isAimPress, const int level, const Vector3& chest, const Quaternion& rotation,
		const Vector3& aimOrigin, const Vector3& aimDirection);
	/** 構えているか（撃っている間と、やめてから aimHoldTime 秒の間） */
	bool IsAiming() const { return m_aimTimer > 0.0f; }
	/** 今ロックしている敵の数（デバッグ画面の表示用） */
	int GetLockCount() const { return static_cast<int>(m_multiLocker.GetTargets().size()); }
	/** ロックの様子（HUD でドーナツを出すため） */
	const MultiLocker& GetMultiLocker() const { return m_multiLocker; }

private:
	/** 照準の先へ向けて弾を1発出す */
	void Shoot(const Vector3& muzzle, const GunParam::Level& levelParam);
	/** 画面の中央の先にある、狙う点を返す */
	Vector3 CalcAimPoint() const;
	/** 銃口の位置を返す（胸から体の右前へずらした点）。壁に埋まるときは胸を返す */
	Vector3 CalcMuzzle(const Vector3& chest, const Quaternion& rotation) const;
	/** ロックした敵へ 1 発ずつホーミング弾を撃つ */
	void FireAtLocked(const Vector3& muzzle, const GunParam::Level& levelParam);

private:
	BulletManager* m_bulletManager = nullptr;				///< 弾を出す先（借りているだけ）
	const GunParam& m_param = nsGameParam::Get().gun;		///< 調整用の値（Assets/param/gun.json）
	float m_shootTimer = 0.0f;								///< 次に撃てるまでの残り時間（秒）
	float m_aimTimer = 0.0f;								///< 構えをやめるまでの残り時間（秒）
	MultiLocker m_multiLocker;								///< LT を押している間に敵をロックする（Lv3 から）
	bool m_wasAimPress = false;								///< 前のフレームに LT を押していたか（離した瞬間を知るため）

	static constexpr float AIM_MIN_DISTANCE = 1.0f;			///< 狙う点が手元からこれより近いと、向きを出せない
};
