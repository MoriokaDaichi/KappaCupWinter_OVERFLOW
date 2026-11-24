#pragma once
#include "hud/HudShape.h"

class BattleCharacter;

/**
 * 自分の弾が敵に当たったときに、照準のまわりへ×の形の線を一瞬出す（ヒットマーク）。倒したときは大きく赤くする
 * プレイヤーの当たった回数・倒した回数を前のフレームと比べて、増えたら出す（GameCamera がワープの回数を見るのと同じ形）。Hud が持つ
 */
class HitMarker
{
public:
	/** 見るプレイヤーを受け取り、線の板を作る（借りているだけ） */
	void Init(const BattleCharacter* player);
	/** 当たった・倒したら出し始め、残り時間を減らす（毎フレーム Hud の Update から呼ぶ） */
	void Update();
	/** 出ている間だけ描く（毎フレーム Hud の Draw から呼ぶ） */
	void Draw();
	/** 消して、数え直す（最初からやり直すとき。プレイヤーが作り直されても、前の数と比べないように） */
	void Reset();

private:
	static constexpr float HIT_TIME = 0.15f;			///< 当たったときのマークが消えるまでの秒数
	static constexpr float DEFEAT_TIME = 0.35f;			///< 倒したときのマークが消えるまでの秒数（長めに残して「倒した」を伝える）
	static constexpr float HIT_SIZE = 64.0f;			///< 当たったときのマークの大きさ（ピクセル）
	static constexpr float DEFEAT_SIZE = 110.0f;		///< 倒したときのマークの大きさ（ピクセル）

	const BattleCharacter* m_player = nullptr;			///< 見るプレイヤー（借りているだけ）
	HudShape m_hitMark;									///< 当たったときのマーク（Assets/shader/hudHitMarker.fx）
	HudShape m_defeatMark;								///< 倒したときのマーク（同じシェーダーで大きく）
	float m_hitTimer = 0.0f;							///< 当たったときのマークの残り時間（秒）
	float m_defeatTimer = 0.0f;							///< 倒したときのマークの残り時間（秒）
	int m_lastHitCount = 0;								///< 前のフレームで見た、当たった回数
	int m_lastDefeatCount = 0;							///< 前のフレームで見た、倒した回数
};
