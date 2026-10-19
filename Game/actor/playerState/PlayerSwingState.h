#pragma once
#include "state/IState.h"

class Player;

/** アンカーにぶら下がって振り子のように動いている状態 */
class PlayerSwingState : public IState<Player>
{
public:
	explicit PlayerSwingState(Player* player) : IState<Player>(player) {}

	/** スイングの準備（アンカーを覚えてワイヤーを飛ばし始める・ジャンプの回数を戻す） */
	void Enter() override;
	/** 離す・ジャンプで Air、着地で Run／Idle に切り替える */
	void Update() override;
	/** スイングの片付け（つかんでいるアンカーを外す） */
	void Exit() override;
};
