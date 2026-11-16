#pragma once
#include "param/GameParam.h"
#include "weapon/MultiLocker.h"

class BulletSystem;
struct BulletShot;

/**
 * RT を押している間、一定の間隔で照準の先へ弾を撃つ。画面の中央の四角に入った敵はいつもロックし、ロック中に RT で撃つとホーミング弾になる。
 * 四角の大きさとロックできる数は、速度 Lv で変わる（gun.json の levels）
 * BattleCharacter が持つ（abilities に "Gun" があるときだけ）
 * プレイヤー専用。敵には EnemyGun を使う。理由:
 * 狙いは、渡された aimDirection ではなく、カメラ（g_camera3D）を直接見ている
 * InitAbilities は、必ずプレイヤーの弾用の "bulletSystem" を渡す（敵に当たる弾）
 */
class Gun
{
public:
	/** 弾を出す BulletSystem と、銃を持つキャラを受け取る（借りているだけ。倒したときの基礎速度は owner に入る） */
	void Init(BulletSystem* bulletSystem, BattleCharacter* owner)
	{
		m_bulletSystem = bulletSystem;
		m_owner = owner;
	}
	/** 
	 * 毎フレーム BattleCharacter が呼ぶ。押していて間隔が空いていれば撃つ。
	 * level は今の速度 Lv（1〜5）。chest は胸の位置、rotation は体の向き（銃口の位置を決める） 
	 * aimOrigin は狙いの始点（カメラの位置。マルチロックオンの距離と壁の判定で使う）
	 */
	void Update(const bool isShootPress, const int level, const Vector3& chest, const Quaternion& rotation, const Vector3& aimOrigin);
	/** 構えているか（撃っている間と、やめてから aimHoldTime 秒の間） */
	bool IsAiming() const { return m_aimTimer > 0.0f; }
	/** 今の Lv でロックする四角の、幅と高さの半分（ピクセル。HUD で同じ大きさの四角を出すため） */
	const Vector2& GetLockBoxHalfSize() const { return m_lockBoxHalfSize; }
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
	/** ロックした敵の数に分裂する弾を 1 発撃つ */
	void FireAtLocked(const Vector3& muzzle, const GunParam::Level& levelParam);
	/** Level から BulletShot を作る */
	BulletShot MakeShot(const GunParam::Level& levelParam) const;

private:
	BulletSystem* m_bulletSystem = nullptr;				///< 弾を出す先（借りているだけ）
	BattleCharacter* m_owner = nullptr;						///< 銃を持つキャラ（借りているだけ）
	const GunParam& m_param = nsGameParam::Get().gun;		///< 調整用の値（Assets/param/gun.json）
	float m_shootTimer = 0.0f;								///< 次に撃てるまでの残り時間（秒）
	float m_aimTimer = 0.0f;								///< 構えをやめるまでの残り時間（秒）
	MultiLocker m_multiLocker;								///< 画面の中央の四角に入った敵をロックする
	Vector2 m_lockBoxHalfSize = Vector2::Zero;				///< 今の Lv でロックする四角の、幅と高さの半分（ピクセル）

	static constexpr float AIM_MIN_DISTANCE = 1.0f;			///< 狙う点が手元からこれより近いと、向きを出せない
	static constexpr float AIM_MIN_DISTANCE_SQ = AIM_MIN_DISTANCE * AIM_MIN_DISTANCE;		///< AIM_MIN_DISTANCEを2乗した値（LengthSq と比べるため）
};
