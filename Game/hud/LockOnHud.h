#pragma once
#include "hud/HudShape.h"
#include "hud/LockOnRing.h"

class BattleCharacter;

/**
 * プレイヤーがロックしかけている敵・ロックした敵の胸の位置に、ドーナツを出す。Game が持つ
 * 画面の中央には、ロックする範囲の四角を出す（大きさは Gun が判定に使うものと同じ）
 */
class LockOnHud
{
public:
	/** 輪と四角を作り、見るプレイヤーを受け取る（借りているだけ） */
	void Init(const BattleCharacter* player);
	/** 毎フレーム Game の Render から呼ぶ（全員が動いた後の位置に出すため） */
	void Draw(RenderContext& rc);

private:
	/** 画面の中央に、今の Lv でロックする範囲の四角を出す */
	void DrawLockBox();
	/** 空いている輪を 1 つ使って描く。輪が足りないときは描かない */
	void DrawRing(RenderContext& rc, const BattleCharacter& target, const float progress, const Vector4& color);

private:
	static constexpr int MAX_RING = 16;					///< 同時に出せる輪の数（ロックの最大 8 体＋ロックしかけ）
	static constexpr float RING_SIZE = 72.0f;			///< 輪の直径（ピクセル）。色は LockOnHud.cpp の上

	std::array<LockOnRing, MAX_RING> m_rings;			///< 輪（1 つの Sprite は 1 フレームに 1 回しか描けないので、出す数だけ持つ）
	int m_usedRingNum = 0;								///< このフレームに使った輪の数
	HudShape m_lockBox;									///< ロックする範囲の四角（Assets/shader/hudLockBox.fx）
	Vector2 m_lockBoxMaxSize = Vector2::Zero;			///< 四角の板の大きさ（全部の Lv の中でいちばん大きい四角。ピクセル）
	const BattleCharacter* m_player = nullptr;			///< 見るプレイヤー（借りているだけ）
};
