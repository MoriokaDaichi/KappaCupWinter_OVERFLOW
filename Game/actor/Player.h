#pragma once
#include "physics/Gravity.h"
#include "state/StateMachine.h"
#include "actor/PlayerState.h"

class InputManager;

/** プレイヤー。移動・ジャンプ・射撃などを行う */
class Player : public IGameObject
{
public:
	Player();
	~Player();
	bool Start();
	void Update();
	void Render(RenderContext& rc);

public:
	/** 入力を受け取る先を設定する。Game が持っている InputManager を渡す */
	void SetInput(const InputManager* input)
	{
		m_input = input;
	}

	/** スティックの向きに移動し、進む方向に体を向ける */
	void Move();
	/** ジャンプする（上向きの速さを入れて、回数を1増やす） */
	void Jump();
	/** まだジャンプできるか（跳んだ回数が上限より少ないか） */
	bool CanJump() const
	{
		return m_jumpCount < MAX_JUMP_COUNT;
	}

private:
	const InputManager* m_input = nullptr;	///< 入力（読むだけ。止めるのは Game の役目）

	static constexpr float MOVE_SPEED = 400.0f;		///< 移動速度用
	static constexpr float JUMP_SPEED = 600.0f;		///< 跳んだ瞬間の上向きの速さ
	static constexpr int MAX_JUMP_COUNT = 2;		///< 何回ジャンプできるか

	CharacterController m_charaCon;	///< 当たり判定と移動
	Gravity m_gravity;				///< 重力
	Vector3 m_moveSpeed;			///< 移動速度
	ModelRender m_modelRender;		///< 見た目
	Quaternion m_rotation;			///< 向き
	int m_jumpCount = 0;			///< いま何回跳んだか
	StateMachine<Player, EnPlayerState> m_stateMachine;		///< 状態の切り替え役
};
