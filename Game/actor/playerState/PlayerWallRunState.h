#pragma once
#include "state/IState.h"
#include "param/GameParam.h"

class BattleCharacter;

/** 壁を走っている状態。壁ジャンプ・着地・壁がなくなるで Air / Run に切り替える */
class PlayerWallRunState : public IState<BattleCharacter>
{
public:
	explicit PlayerWallRunState(BattleCharacter* owner);

	/** 壁走りの準備（落ちる速さを消す・重力を弱める・ジャンプの回数を戻す） */
	void Enter() override;
	/** 壁に沿って走る。ジャンプで壁ジャンプ、着地で Run、壁がなくなったら Air に切り替える */
	void Update() override;
	/** 壁走りの片付け（重力を戻す・しばらく同じ壁に張り付けないようにする） */
	void Exit() override;

	/** 左右にレイを飛ばして壁を探す。見つかったら outNormal に壁の向きを入れて true を返す */
	static bool FindWall(const BattleCharacter& owner, Vector3& outNormal);

private:
	/** 壁に沿って走る。時間内は落ちず、過ぎたらずり落ちる */
	void Move();
	/** 壁から離れる向きへ体を傾けるよう、体に伝える */
	void UpdateTilt(const Vector3& wallNormal);
	/** 壁からジャンプで離れる */
	void WallJump();

private:
	static constexpr float TILT_MIN_SIN = 0.1f;				///< 壁走りの傾きの中心を出すときの割る数の下限（角度が 0 に近いと中心が無限に高くなるため）

	Vector3 m_wallNormal = Vector3::Zero;					///< 今の壁の向き（法線）。FindWall で書き換える
	float m_runTime = 0.0f;				///< 壁を走り始めてからの時間
	const CharacterParam::WallRun& m_param;		///< 壁走りの値（持ち主の設定から受け取る）
	const CharacterParam::Body& m_bodyParam;	///< 体の大きさの値（同上）
};
