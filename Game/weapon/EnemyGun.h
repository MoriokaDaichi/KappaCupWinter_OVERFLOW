#pragma once
#include "weapon/WarningLine.h"

class BulletSystem;

/**
 * 撃つコマンドが true のフレームに、狙いの向きへ1発撃つ。構えるコマンドが true の間は、予告線を出す
 * BattleCharacter が持つ（abilities に "EnemyGun" があるときだけ）
 */
class EnemyGun
{
public:
	/** 弾を出す先を受け取り（借りているだけ。Game が持つ敵用の BulletSystem）、予告線を用意する */
	void Init(BulletSystem* bulletSystem);
	/**
	 * 毎フレーム 敵の BattleCharacter が呼ぶ。isShootPress が true のフレームごとに1発。間隔は AI の休む時間で決まる。
	 * muzzle・aimDirection は弾を出す位置（砲口）と向き（長さは何でもよい。中で長さ1にする）
	 */
	void Update(const bool isShootPress, const Vector3& muzzle, const Vector3& aimDirection);
	/**
	 * 予告線を動かす。isAimPress が true の間だけ出す。muzzle・aimDirection は Update と同じ。aimDistance は狙う点までの距離（0 なら弾が届く所まで）
	 * 砲身の向きが決まった後（BattleCharacter の OnBodyUpdated）に呼ぶ。先に呼ぶと、線が 1 フレーム前の砲口から出て、砲身からずれて見える
	 */
	void UpdateWarningLine(const bool isAimPress, const Vector3& muzzle, const Vector3& aimDirection, const float aimDistance);
	/** 予告線を描く（持ち主の Render から呼ぶ） */
	void Draw(RenderContext& rc);

private:
	BulletSystem* m_bulletSystem = nullptr;		///< 弾を出す先（借りているだけ）
	WarningLine m_warningLine;					///< 狙っている間に出す予告線
};
