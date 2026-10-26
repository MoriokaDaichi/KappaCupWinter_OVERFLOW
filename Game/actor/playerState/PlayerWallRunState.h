#pragma once
#include "state/IState.h"
#include "param/GameParam.h"

class Player;

/** 壁を走っている状態。壁ジャンプ・着地・壁がなくなるで Air / Run に切り替える */
class PlayerWallRunState : public IState<Player>
{
public:
	explicit PlayerWallRunState(Player* player) : IState<Player>(player) {}

	/** 壁走りの準備（落ちる速さを消す・重力を弱める・ジャンプの回数を戻す） */
	void Enter() override;
	/** 壁に沿って走る。ジャンプで壁ジャンプ、着地で Run、壁がなくなったら Air に切り替える */
	void Update() override;
	/** 壁走りの片付け（重力を戻す・しばらく同じ壁に張り付けないようにする） */
	void Exit() override;

private:
	/** 壁に沿って走る。時間内は落ちず、過ぎたらずり落ちる */
	void Move();
	/** 壁から離れる向きへ体を傾けるよう、体に伝える */
	void UpdateTilt(const Vector3& wallNormal);

private:
	static constexpr float TILT_MIN_SIN = 0.1f;				///< 壁走りの傾きの中心を出すときの割る数の下限（角度が 0 に近いと中心が無限に高くなるため）

	float m_runTime = 0.0f;				///< 壁を走り始めてからの時間
	const PlayerParam::WallRun& m_param = nsGameParam::Get().player.wallRun;		///< 調整用の値のうち、壁走りの分（Assets/param/player.json）
	const PlayerParam::Body& m_bodyParam = nsGameParam::Get().player.body;			///< 調整用の値のうち、体の大きさの分（Assets/param/player.json）
};
