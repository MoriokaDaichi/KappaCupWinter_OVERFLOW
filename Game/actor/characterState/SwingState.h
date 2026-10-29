#pragma once
#include "actor/characterState/CharacterState.h"
#include "param/GameParam.h"
#include <limits>

class BattleCharacter;
class Anchor;

/** アンカーにぶら下がって振り子のように動いている状態 */
class SwingState : public CharacterState
{
public:
	explicit SwingState(BattleCharacter* owner);

	/** スイングの準備（アンカーを覚えてワイヤーを飛ばし始める・ジャンプの回数を戻す） */
	void Enter() override;
	/** ワイヤーを飛ばす・振り子で動く（切り替えは切り替え表が先に見る） */
	void Update() override;
	/** スイングの片付け（つかんでいるアンカーを外す） */
	void Exit() override;
	/** 切り替え表の条件のうち、スイングだけが判定できるもの（Hooked）に答える */
	bool IsConditionMet(const EnCondition when) const override;
	/** 切り替え表の動作のうち、スイングだけができるもの（ReleaseBoost）をする */
	void DoAction(const EnTransitionAction action) override;

private:
	/** ワイヤーが届くまでは飛ばす。届いたら巻き上げながら、アンカーを中心に振り子のように動く */
	void Move();
	/** ワイヤーの先を手元からアンカーへ飛ばす。届いたらつかむ。飛んでいる間は普通に動く */
	void ShootWire();
	/** ワイヤーの先が届いたときに、アンカーをつかむ（ロープの長さを決める） */
	void HookAnchor(const float ropeLength);
	/** スティックの向きのうち、ロープと直角な向きにだけ加速する（こぐ） */
	void SwingPump(const Vector3& ropeDir);
	/** 速さが上限を超えていたら、向きはそのまま長さを上限にそろえる */
	void ClampSwingSpeed();
	/** スイングを離したときに、進んでいる向きと上に速さを足す（つかんでいて、十分速いときだけ） */
	void ReleaseBoost();
	/** 壁から離れる向きへ体を傾けるよう、体に伝える */
	void UpdateTilt();

private:
	static constexpr float SWING_MIN_DISTANCE = 0.001f;		///< アンカーとの距離がこれより近いと、ロープの向きを出せない
	static constexpr float WIRE_HOOKED_LENGTH = (std::numeric_limits<float>::max)();	///< つかんだ後にワイヤーへ渡す先の長さ（必ずアンカーまで届く）
	static constexpr float PUMP_MIN_INPUT = 0.001f;			///< ロープの向きを除いたスティックの残りがこれより短い（長さの2乗）と、こがない

	const Anchor* m_swingAnchor = nullptr;		///< スイングに使うアンカー（借りているだけ）
	const CharacterParam::Swing& m_param;		///< スイングの値（持ち主の設定から受け取る）
	const CharacterParam::Body& m_bodyParam;	///< 体の大きさの値（同上）
	float m_ropeLength = 0.0f;					///< ロープの長さ。つかんだときの距離を入れ、巻き上げで縮める
	float m_wireTipLength = 0.0f;				///< 手元から飛んでいるワイヤーの先までの長さ
	bool m_isHooked = false;					///< ワイヤーの先がアンカーに届いたか
};
