#pragma once
#include "actor/Character.h"
#include "state/StateMachine.h"
#include "actor/playerState/PlayerState.h"

class InputManager;

/** プレイヤー。移動・ジャンプ・射撃などを行う */
class Player : public Character
{
public:
	Player();
	~Player();
	bool Start();

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
	bool CanJump() const { return m_jumpCount < MAX_JUMP_COUNT;}
	/** ジャンプが押されたか */
	bool IsJumpTrigger() const;
	/** スティックが倒れているか */
	bool IsMoveInput() const;
	/** 状態を切り替える */
	void ChangeState(EnPlayerState id);
	/** ジャンプの回数を 0 に戻す */
	void ResetJumpCount();
	/** 横の速さを 0 にする（縦の速さはそのまま） */
	void StopHorizontal();
	/** 下に向かっているか */
	bool IsFalling() const;
	/** 地上のジャンプを使ったことにする（まだ跳んでいなければ回数を 1 にする） */
	void ConsumeGroundJump();
	/** 空中での移動。スティックの向きへ少しずつ曲がる（離すと勢いのまま） */
	void AirMove();
	/** スティックとカメラから進行方向を決める */
	Vector3 GetMoveDirection() const;

protected:
	/** プレイヤーの行動（ステートマシンを動かす） */
	void UpdateAction() override;

private:
	const InputManager* m_input = nullptr;	///< 入力（読むだけ。止めるのは Game の役目）
	StateMachine<Player, EnPlayerState> m_stateMachine;		///< 状態の切り替え役

	static constexpr float MOVE_SPEED = 400.0f;		///< 移動速度用
	static constexpr float JUMP_SPEED = 600.0f;		///< 跳んだ瞬間の上向きの速さ
	static constexpr int MAX_JUMP_COUNT = 2;		///< 何回ジャンプできるか
	static constexpr float MOVE_INPUT_THRESHOLD = 0.001f;	///< スティックを倒したとみなす境目（長さの2乗）
	static constexpr float AIR_CONTROL = 1.5f;		///< 空中移動について1秒あたりに近づく割合

	int m_jumpCount = 0;			///< いま何回跳んだか
};
