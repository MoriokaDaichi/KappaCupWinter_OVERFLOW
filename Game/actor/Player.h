#pragma once
#include "actor/Character.h"
#include "state/StateMachine.h"
#include "actor/playerState/PlayerState.h"
#include "param/GameParam.h"

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
	bool CanJump() const { return m_jumpCount < m_param.jump.maxCount;}
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
	/** スイングを離したときに、進んでいる向きと上に速さを足す（つかんでいて、十分速いときだけ） */
	void ReleaseBoost();

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
	/** 速さが上限を超えていたら、向きはそのまま長さを上限にそろえる */
	void ClampSwingSpeed();

private:
	const InputManager* m_input = nullptr;	///< 入力（読むだけ。止めるのは Game の役目）
	StateMachine<Player, EnPlayerState> m_stateMachine;		///< 状態の切り替え役
	Anchor* m_lockedAnchor = nullptr;		///< ロック中のアンカー
	Anchor* m_swingAnchor = nullptr;		///< スイングに使うアンカー
	GrappleWire* m_grappleWire = nullptr;	///< ワイヤーの見た目。スイング中だけ表示する

	// 調整する値は Assets/param/player.json（意味は param/GameParam.h の PlayerParam）。ここには計算のための値だけ置く
	static constexpr float MOVE_INPUT_THRESHOLD = 0.001f;	///< スティックを倒したとみなす境目（長さの2乗）
	static constexpr float SWING_MIN_DISTANCE = 0.001f;		///< アンカーとの距離がこれより近いと、ロープの向きを出せない

	const PlayerParam& m_param = nsGameParam::Get().player;	///< 調整用の値（F5 で読み直すと中身が変わる）

	int m_jumpCount = 0;			///< いま何回跳んだか
	Vector3 m_wallNormal = Vector3::Zero;		///< 今の壁の向き（法線）。FindWall で書き換える
	float m_wallRunTimer = 0.0f;	///< 壁を走り始めてからの時間
	float m_reattachTimer = 0.0f;	///< もう一度張り付けるまでの残り時間
	float m_ropeLength = 0.0f;		///< ロープの長さ。つかんだときの距離を入れ、巻き上げで縮める
	float m_wireTipLength = 0.0f;	///< 手元から飛んでいるワイヤーの先までの長さ
	bool m_isHooked = false;		///< ワイヤーの先がアンカーに届いたか
};
