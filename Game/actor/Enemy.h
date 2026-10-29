#pragma once
#include "actor/BattleCharacter.h"

/** 敵。BattleCharacter に体のモデルを付けただけ（仮: ステップ 5 の確認用。種類ごとの違いはステップ 6 の JSON で決める） */
class Enemy : public BattleCharacter
{
public:
	bool Start();

public:
	/** 出てくる位置を決める（Start より前に呼ぶ） */
	void SetStartPosition(const Vector3& position) { m_startPosition = position; }

private:
	Vector3 m_startPosition = Vector3::Zero;		///< 出てくる位置（足元）
	const PlayerParam& m_param = nsGameParam::Get().player;	///< 体の大きさ。仮: ステップ 6 で敵ごとの設定から読む
};
