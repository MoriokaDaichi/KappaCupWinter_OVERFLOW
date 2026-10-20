#pragma once
#include "actor/Character.h"
#include "state/StateMachine.h"
#include "actor/playerState/PlayerState.h"

class InputManager;
class Anchor;
class GrappleWire;

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
	/** 横の速さを target に、1秒あたり rate だけ近づける */
	void ApproachHorizontal(const Vector3& target, float rate);
	/** 止まるまで減速する（Idle から呼ぶ） */
	void Decelerate();
	/** スイングに使うロック中のアンカーを返す */
	Anchor* GetLockedAnchor() const { return m_lockedAnchor; }
	/** グラップルが押されたか */
	bool IsGrappleTrigger() const;
	/** グラップルを離したか */
	bool IsGrappleRelease() const;
	/** スイングを始められるか（ロック中のアンカーがあるか） */
	bool CanStartSwing() const {return m_lockedAnchor != nullptr; }
	/** スイングを始めるときの準備（つかむアンカーを覚え、ワイヤーを飛ばし始める） */
	void StartSwing();
	/** ワイヤーが届くまでは飛ばす。届いたら巻き上げながら、アンカーを中心に振り子のように動く */
	void SwingMove();
	/** スイングを終わるときの片付け（つかんでいるアンカーを外す） */
	void EndSwing();
	/** ワイヤーの先がアンカーに届いたか（届くまでは飛んでいる途中） */
	bool IsGrappleHooked() const { return m_isHooked; }

protected:
	/** プレイヤーの行動（ステートマシンを動かす） */
	void UpdateAction() override;

private:
	/** 毎フレーム、一番いいアンカーを選んで m_lockedAnchor に入れる */
	void UpdateLock();
	/** アンカー1つの点数を出す。候補にならないときはマイナス（例：-1.0f）を返す */
	float CalcLockScore(const Anchor* anchor) const;
	/** ロープを出す点を返す */
	Vector3 GetRopeOrigin() const;
	/** ワイヤーの先を手元からアンカーへ飛ばす。届いたらつかむ。飛んでいる間は普通に動く */
	void ShootWire();
	/** ワイヤーの先が届いたときに、アンカーをつかむ（ロープの長さを決める） */
	void HookAnchor(float ropeLength);
	/** スティックの向きのうち、ロープと直角な向きにだけ加速する（こぐ） */
	void SwingPump(const Vector3& ropeDir);

private:
	const InputManager* m_input = nullptr;	///< 入力（読むだけ。止めるのは Game の役目）
	StateMachine<Player, EnPlayerState> m_stateMachine;		///< 状態の切り替え役
	Anchor* m_lockedAnchor = nullptr;		///< ロック中のアンカー
	Anchor* m_swingAnchor = nullptr;		///< スイングに使うアンカー
	GrappleWire* m_grappleWire = nullptr;	///< ワイヤーの見た目。スイング中だけ表示する

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
	static constexpr float GROUND_ACCEL = 3000.0f;			///< 地上で加速する量（1秒あたり）
	static constexpr float GROUND_DECEL = 2000.0f;			///< 地上で減速する量（1秒あたり）
	static constexpr float LOCK_MAX_DISTANCE = 3000.0f;		///< ロックできる距離
	static constexpr float LOCK_MAX_ANGLE = 30.0f;			///< 画面の中央からの角度の上限（度）
	static constexpr float LOCK_ANGLE_WEIGHT = 0.7f;		///< 角度の点の重み
	static constexpr float LOCK_DISTANCE_WEIGHT = 0.3f;		///< 距離の点の重み
	static constexpr float LOCK_KEEP_BONUS = 0.1f;			///< 今ロックしているアンカーに足すおまけの点
	static constexpr float SWING_START_LIFT = 100.0f;		///< 地上でつかんだときに浮かせる上向きの速さ
	static constexpr float SWING_PULL_RATE = 5.0f;			///< ロープが伸びた分を、1秒あたりに戻す割合
	static constexpr float SWING_MIN_DISTANCE = 0.001f;		///< アンカーとの距離がこれより近いと、ロープの向きを出せない
	static constexpr float WIRE_SHOOT_SPEED = 8000.0f;		///< ワイヤーの先が飛んでいく速さ（1秒あたり。3000 先まで約 0.4 秒）
	static constexpr float SWING_REEL_SPEED = 1000.0f;		///< 巻き上げでアンカーへ引き寄せる速さ（1秒あたり）
	static constexpr float SWING_MIN_ROPE_LENGTH = 300.0f;	///< 巻き上げを止めるロープの長さ。ここからは振り子だけ
	static constexpr float SWING_REEL_EASE = 3.0f;			///< 最短の長さまでの残りに掛けて巻き上げの速さを決める（近いほど遅い）
	static constexpr float SWING_PUMP_ACCEL = 800.0f;		///< グラップルで漕いだ時の加速（1秒あたり）
	static constexpr float SWING_FACE_MIN_SPEED = 50.0f;	///< スイング中に体の向きを変える、横の速さの下限

	int m_jumpCount = 0;			///< いま何回跳んだか
	Vector3 m_wallNormal = Vector3::Zero;		///< 今の壁の向き（法線）。FindWall で書き換える
	float m_wallRunTimer = 0.0f;	///< 壁を走り始めてからの時間
	float m_reattachTimer = 0.0f;	///< もう一度張り付けるまでの残り時間
	float m_ropeLength = 0.0f;		///< ロープの長さ。つかんだときの距離を入れ、巻き上げで縮める
	float m_wireTipLength = 0.0f;	///< 手元から飛んでいるワイヤーの先までの長さ
	bool m_isHooked = false;		///< ワイヤーの先がアンカーに届いたか
};
