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
	/** 左右にレイを飛ばして壁を探す。見つかったら m_wallNormal に入れて true を返す */
	bool FindWall();
	/** 壁走りを始められるか */
	bool CanStartWallRun() const;
	/** 壁に沿って走る */
	void WallRunMove();
	/** 壁から跳ねる */
	void WallJump();
	/** 壁走りを始めるときの準備 */
	void StartWallRun();
	/** 壁走りを終わるときの片付け */
	void EndWallRun();
	/** 落ちずに走れる時間を過ぎたか */
	bool IsWallRunTimeOver() const;

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
	static constexpr float WALL_CHECK_LENGTH = 50.0f;		///< 壁を探すレイの長さ
	static constexpr float WALL_CHECK_HEIGHT = 50.0f;		///< 壁を探すレイを飛ばす位置を作る時に m_position に足す値
	static constexpr float WALL_NORMAL_Y_LIMIT = 0.3f;		///< 壁の向き（法線）の y の絶対値がこれより小さければ壁とみなす
	static constexpr float WALLRUN_SPEED = 600.0f;			///< 壁を走る速さ
	static constexpr float WALLRUN_MIN_SPEED = 200.0f;		///< 壁走りを始めるのに必要な横の速さ
	static constexpr float WALLRUN_GRAVITY_SCALE = 0.15f;	///< 壁走り中の重力の倍率
	static constexpr float WALLRUN_STICK_SPEED = 50.0f;		///< 壁に押し付ける速さ
	static constexpr float WALLRUN_MAX_TIME = 2.0f;			///< 落ちずに壁を走れる時間（秒）。過ぎるとゆっくりずり落ちる
	static constexpr float WALL_JUMP_SPEED_UP = 600.0f;		///< 壁ジャンプで上に跳ぶ速さ
	static constexpr float WALL_JUMP_SPEED_OUT = 400.0f;	///< 壁ジャンプで壁から離れる速さ
	static constexpr float WALL_REATTACH_TIME = 0.3f;		///< 壁を離れてから、もう一度張り付けるまでの時間（秒）

	int m_jumpCount = 0;			///< いま何回跳んだか
	Vector3 m_wallNormal = Vector3::Zero;		///< 今の壁の向き（法線）。FindWall で書き換える
	float m_wallRunTimer = 0.0f;	///< 壁を走り始めてからの時間
	float m_reattachTimer = 0.0f;	///< もう一度張り付けるまでの残り時間
};
