#pragma once
#include "state/IState.h"

class Player;

/**	走っている状態。入力や接地を見て Air / Idle を切り替える */
class PlayerRunState : public IState<Player>
{
public:
	explicit PlayerRunState(Player* player) : IState<Player>(player) {}

	/** ジャンプの回数を戻す */
	void Enter() override;
	/** 入力と接地を見て Air / Idle に切り替える。どちらでもなければ走る */
	void Update() override;
};
