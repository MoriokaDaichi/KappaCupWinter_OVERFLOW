#pragma once
#include "actor/characterState/CharacterState.h"
#include "param/GameParam.h"

class BattleCharacter;

/** 壁を走っている状態 */
class WallRunState : public CharacterState
{
public:
	explicit WallRunState(BattleCharacter* owner);

	/** 壁走りの準備（落ちる速さを消す・重力を弱める・ジャンプの回数を戻す） */
	void Enter() override;
	/** 壁の向きを取り直して、壁に沿って走る（切り替えは切り替え表が先に見る） */
	void Update() override;
	/** 壁走りの片付け（重力を戻す・しばらく同じ壁に張り付けないようにする） */
	void Exit() override;
	/** 切り替え表の動作のうち、壁走りだけができるもの（WallJump）をする */
	void DoAction(const EnTransitionAction action) override;

	/** 左右にレイを飛ばして壁を探す。見つかったら outNormal に壁の向きを入れて true を返す（切り替え表の WallFound・WallLost の判定からも呼ぶ） */
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
