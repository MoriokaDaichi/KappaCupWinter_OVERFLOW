#pragma once
#include "actor/Character.h"
#include "state/StateMachine.h"
#include "actor/playerState/PlayerState.h"
#include "weapon/Gun.h"
#include "param/GameParam.h"
#include "speed/SpeedSystem.h"

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
	/** 射撃ボタンを押しているか */
	bool IsShootPress() const;
	/** エイムボタンを押しているか */
	bool IsAimPress() const;
	/** 速度を返す（HUD やデバッグ表示で読む） */
	const SpeedSystem& GetSpeedSystem() const { return m_speed; }
	/** 速度を返す（書き換えられる。デバッグ画面用） */
	SpeedSystem& GetSpeedSystem() { return m_speed; }
	/** 今の状態を返す（デバッグ画面の表示用） */
	EnPlayerState GetCurrentState() const { return m_stateMachine.GetCurrentState(); }
	/** position へ瞬間移動する。速さを 0 にして空中の状態から始める（デバッグ画面・チェックポイント用） */
	void Warp(const Vector3& position);

protected:
	/** プレイヤーの行動（ステートマシンを動かす） */
	void UpdateAction() override;
	/** 体を動かした後（骨の位置が決まった後）に、ワイヤーの見た目を合わせる */
	void OnBodyUpdated() override;

private:
	/** 毎フレーム、一番いいアンカーを選んで m_lockedAnchor に入れる */
	void UpdateLock();
	/** アンカー1つの点数を出す。候補にならないときはマイナス（例：-1.0f）を返す */
	float CalcLockScore(const Anchor* anchor) const;
	/** 胸の位置を返す。ロープの手元と、銃口を決める基準に使う */
	Vector3 GetChestPosition() const;
	/** ワイヤーの先を手元からアンカーへ飛ばす。届いたらつかむ。飛んでいる間は普通に動く */
	void ShootWire();
	/** ワイヤーの先が届いたときに、アンカーをつかむ（ロープの長さを決める） */
	void HookAnchor(float ropeLength);
	/** スティックの向きのうち、ロープと直角な向きにだけ加速する（こぐ） */
	void SwingPump(const Vector3& ropeDir);
	/** 速さが上限を超えていたら、向きはそのまま長さを上限にそろえる */
	void ClampSwingSpeed();
	/** 構えている間、体をカメラの前向きに向ける */
	void FaceAimDirection();
	/** 今の状態に合わせて、上乗せの速さを増やす・減らす（壁走りで増え、地上で減る） */
	void UpdateSpeedBonus();
	/** アニメーションを読み込む（InitCharacter の前に呼ぶ） */
	void LoadAnimation();
	/** 状態に合ったアニメーションに切り替える */
	void PlayStateAnimation(EnPlayerState state);
	/** アニメーションを切り替える。同じアニメーションなら、isRestart が true のときだけ最初から流し直す */
	void PlayAnimation(EnPlayerAnim anim, bool isRestart = false);
	/** 空中にいる間は速さに合わせて切り替え、着地のアニメーションが終わったら待機につなぐ */
	void UpdateAnimation();
	/** 空中のアニメーションを選ぶ。踏み切りの途中ならそのまま、速く落ちていれば FallFast、それ以外は Fall */
	EnPlayerAnim SelectAirAnimation() const;
	/** ロープを持つ手の位置（両手の間）。骨が見つからないときは胸の位置 */
	Vector3 GetRopeHandPosition() const;
	/** 体の見た目を傾ける。スイングではアンカーの方へ、壁走りでは壁から離れる向きへ。ほかの状態ではまっすぐに戻す */
	void UpdateBodyTilt();
	/** ワイヤーの見た目を、手からアンカー（飛んでいる途中は先まで）に合わせる */
	void UpdateWireEnds();

private:
	const InputManager* m_input = nullptr;	///< 入力（読むだけ。止めるのは Game の役目）
	StateMachine<Player, EnPlayerState> m_stateMachine;		///< 状態の切り替え役
	Anchor* m_lockedAnchor = nullptr;		///< ロック中のアンカー
	Anchor* m_swingAnchor = nullptr;		///< スイングに使うアンカー
	GrappleWire* m_grappleWire = nullptr;	///< ワイヤーの見た目。スイング中だけ表示する
	Gun m_gun;								///< 銃。RT で弾を撃つ
	SpeedSystem m_speed;					///< スピードシステム（基礎速度と上乗せ速度をもつ）
	std::array<AnimationClip, static_cast<int>(EnPlayerAnim::Num)> m_animClips;	///< アニメーション（ModelRender が借りるので、ここで持つ）
	EnPlayerAnim m_currentAnim = EnPlayerAnim::Num;	///< 今流しているアニメーション（Num はまだ流していない）
	float m_airFallSpeed = 0.0f;			///< 空中で最後に見た、落ちる速さ（着地すると速さが 0 になるので、着地のアニメーションを流すかの判定用に覚えておく）
	int m_leftHandBoneId = -1;				///< 左手の骨の番号（ロープの手元に使う。見つからなければ -1）
	int m_rightHandBoneId = -1;				///< 右手の骨の番号

	// 調整する値は Assets/param/player.json（意味は param/GameParam.h の PlayerParam）。ここには計算のための値だけ置く
	static constexpr float MOVE_INPUT_THRESHOLD = 0.001f;	///< スティックを倒したとみなす境目（長さの2乗）
	static constexpr float SWING_MIN_DISTANCE = 0.001f;		///< アンカーとの距離がこれより近いと、ロープの向きを出せない
	static constexpr float TILT_MIN_SIN = 0.1f;				///< 壁走りの傾きの中心を出すときの割る数の下限（角度が 0 に近いと中心が無限に高くなるため）
	static constexpr float ANIM_INTERPOLATE_TIME = 0.2f;	///< アニメーションを切り替えるときに、前のものとなめらかにつなぐ時間（秒）。調整は遷移のタスクで JSON に出す

	const PlayerParam& m_param = nsGameParam::Get().player;	///< 調整用の値（F5 で読み直すと中身が変わる）
	const SpeedParam& m_speedParam = nsGameParam::Get().speed;		///< 速度の調整値（speed.json）

	int m_jumpCount = 0;			///< いま何回跳んだか
	Vector3 m_wallNormal = Vector3::Zero;		///< 今の壁の向き（法線）。FindWall で書き換える
	float m_wallRunTimer = 0.0f;	///< 壁を走り始めてからの時間
	float m_reattachTimer = 0.0f;	///< もう一度張り付けるまでの残り時間
	float m_ropeLength = 0.0f;		///< ロープの長さ。つかんだときの距離を入れ、巻き上げで縮める
	float m_wireTipLength = 0.0f;	///< 手元から飛んでいるワイヤーの先までの長さ
	bool m_isHooked = false;		///< ワイヤーの先がアンカーに届いたか
};
