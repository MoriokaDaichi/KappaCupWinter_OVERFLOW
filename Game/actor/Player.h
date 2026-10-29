#pragma once
#include "actor/BattleCharacter.h"
#include "state/StateMachine.h"
#include "actor/playerState/PlayerState.h"
#include "weapon/Gun.h"
#include "weapon/GunModel.h"
#include "param/GameParam.h"
#include "speed/SpeedSystem.h"
#include "actor/CharacterCommand.h"
#include "grapple/Grapple.h"
#include <array>

class Anchor;

/** プレイヤー。移動・ジャンプ・射撃などを行う */
class Player : public BattleCharacter
{
public:
	Player();
	bool Start();

public:
	/** スティックの向きに移動し、進む方向に体を向ける */
	void Move();
	/** ジャンプする（上向きの速さを入れて、回数を1増やす） */
	void Jump();
	/** まだジャンプできるか（跳んだ回数が上限より少ないか） */
	bool CanJump() const { return m_jumpCount < m_param.jump.maxCount;}
	/** 状態を切り替える */
	void ChangeState(const EnPlayerState id);
	/** ジャンプの回数を 0 に戻す */
	void ResetJumpCount();
	/** ジャンプの回数を 1 増やす */
	void AddJumpCount();
	/** 地上のジャンプを使ったことにする（まだ跳んでいなければ回数を 1 にする） */
	void ConsumeGroundJump();
	/** 空中での移動。スティックの向きへ少しずつ曲がる（離すと勢いのまま） */
	void AirMove();
	/** 壁走りを始められるか */
	bool CanStartWallRun() const;
	/** 止まるまで減速する（Idle から呼ぶ） */
	void Decelerate();
	/** スイングに使うロック中のアンカーを返す */
	Anchor* GetLockedAnchor() const { return m_grapple.GetLockedAnchor(); }
	/** スイングを始められるか（ロック中のアンカーがあるか） */
	bool CanStartSwing() const {return m_grapple.GetLockedAnchor() != nullptr; }
	/** 速度を返す（HUD やデバッグ表示で読む） */
	const SpeedSystem& GetSpeedSystem() const { return m_speed; }
	/** 速度を返す（書き換えられる。デバッグ画面用） */
	SpeedSystem& GetSpeedSystem() { return m_speed; }
	/** 今の状態を返す（デバッグ画面の表示用） */
	EnPlayerState GetCurrentState() const { return m_stateMachine.GetCurrentState(); }
	/** position へ瞬間移動する。速さを 0 にして空中の状態から始める（デバッグ画面・チェックポイント用） */
	void Warp(const Vector3& position);
	/** 胸の位置を返す。ロープの手元と、銃口を決める基準に使う */
	Vector3 GetChestPosition() const;
	/** グラップルの能力を返す（スイングの State がワイヤーに使う） */
	Grapple& GetGrapple() { return m_grapple; }

protected:
	/** プレイヤーの行動（ステートマシンを動かす） */
	void UpdateAction() override;
	/** 体を動かした後（骨の位置が決まった後）に、ワイヤーと銃の見た目を合わせる */
	void OnBodyUpdated() override;
	/** 体と、手に持つ銃を描く */
	void Render(RenderContext& rc) override;

private:
	/** 構えている間、体をカメラの前向きに向ける */
	void FaceAimDirection();
	/** 今の状態に合わせて、上乗せの速さを増やす・減らす（壁走りで増え、地上で減る） */
	void UpdateSpeedBonus();
	/** アニメーションを読み込む（InitCharacter の前に呼ぶ） */
	void LoadAnimation();
	/** 状態に合ったアニメーションに切り替える */
	void PlayStateAnimation(const EnPlayerState state);
	/** アニメーションを切り替える。同じアニメーションなら、isRestart が true のときだけ最初から流し直す */
	void PlayAnimation(const EnPlayerAnim anim, const bool isRestart = false);
	/** 空中にいる間は速さに合わせて切り替え、着地のアニメーションが終わったら待機につなぐ */
	void UpdateAnimation();
	/** 空中のアニメーションを選ぶ。踏み切りの途中ならそのまま、速く落ちていれば FallFast、それ以外は Fall */
	EnPlayerAnim SelectAirAnimation() const;
	/** ロープを持つ手の位置（両手の間）。骨が見つからないときは胸の位置 */
	Vector3 GetRopeHandPosition() const;
	/** 体の見た目を傾ける。スイングではアンカーの方へ、壁走りでは壁から離れる向きへ。ほかの状態ではまっすぐに戻す */
	void UpdateBodyTilt();
	/** ワイヤーの手元を渡す（骨が動いた後に呼ぶ） */
	void UpdateWireHand();
	/** 銃の見た目を右手に持たせる（握る位置は、手首と中指の付け根の間＝手のひら） */
	void UpdateGunModel();

private:
	StateMachine<Player, EnPlayerState> m_stateMachine;		///< 状態の切り替え役
	Grapple m_grapple;						///< グラップルの能力（狙うアンカーの選択とワイヤーの見た目）
	Gun m_gun;								///< 銃。RT で弾を撃つ
	GunModel m_gunModel;					///< 右手に持つ銃の見た目
	SpeedSystem m_speed;					///< スピードシステム（基礎速度と上乗せ速度をもつ）
	std::array<AnimationClip, static_cast<int>(EnPlayerAnim::Num)> m_animClips;	///< アニメーション（ModelRender が借りるので、ここで持つ）
	EnPlayerAnim m_currentAnim = EnPlayerAnim::Num;	///< 今流しているアニメーション（Num はまだ流していない）
	float m_airFallSpeed = 0.0f;			///< 空中で最後に見た、落ちる速さ（着地すると速さが 0 になるので、着地のアニメーションを流すかの判定用に覚えておく）
	int m_leftHandBoneId = -1;				///< 左手の骨の番号（ロープの手元に使う。見つからなければ -1）
	int m_rightHandBoneId = -1;				///< 右手の骨の番号
	int m_rightMiddleBoneId = -1;			///< 右手の中指の付け根の骨の番号（銃を握る位置に使う）

	// 調整する値は Assets/param/player.json（意味は param/GameParam.h の PlayerParam）。ここには計算のための値だけ置く
	static constexpr float ANIM_INTERPOLATE_TIME = 0.2f;	///< アニメーションを切り替えるときに、前のものとなめらかにつなぐ時間（秒）。調整は遷移のタスクで JSON に出す
	static constexpr float START_HEIGHT = 300.0f;			///< 始めるときに置く高さ（床の少し上から落とす）

	const PlayerParam& m_param = nsGameParam::Get().player;	///< 調整用の値（F5 で読み直すと中身が変わる）
	const SpeedParam& m_speedParam = nsGameParam::Get().speed;		///< 速度の調整値（speed.json）

	int m_jumpCount = 0;			///< いま何回跳んだか
};
