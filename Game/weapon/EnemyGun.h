#pragma once

class BulletSystem;

/**
 * 撃つコマンドが true のフレームに、狙いの向きへ1発撃つ。
 * BattleCharacter が持つ（abilities に "EnemyGun" があるときだけ）
 */
class EnemyGun
{
public:
	/** 弾を出す先を受け取る（借りているだけ。Game が持つ敵用の BulletSystem） */
	void Init(BulletSystem* bulletSystem);
	/**
	 * 毎フレーム 敵の BattleCharacter が呼ぶ。isShootPress が true のフレームごとに1発。間隔は AI の休む時間で決まる。
	 * muzzle・aimDirection は弾を出す位置（砲口）と向き（長さは何でもよい。中で長さ1にする）
	 */
	void Update(const bool isShootPress, const Vector3& muzzle, const Vector3& aimDirection);

private:
	BulletSystem* m_bulletSystem = nullptr;		///< 弾を出す先（借りているだけ）
};
