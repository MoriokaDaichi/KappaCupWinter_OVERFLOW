#pragma once
#include "state/IState.h"

class Player;

/** 空中にいる状態。入力や接地を見て Run / Idle / WallRun に切り替える */
class PlayerAirState : public IState<Player>
{
public:
	explicit PlayerAirState(Player* player) : IState<Player>(player) {}

	/** 足場から落ちてきたときは、地上のジャンプを使ったことにする */
	void Enter() override;
	/** 着地したら Run / Idle、壁に触れたら WallRun に切り替える。空中では 2 段目のジャンプ・移動・重力を行う */
	void Update() override;
};
