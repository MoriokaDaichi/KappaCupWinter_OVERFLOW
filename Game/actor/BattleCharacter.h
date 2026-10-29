#pragma once
#include "actor/Character.h"
#include "actor/CharacterCommand.h"
#include "grapple/Grapple.h"
#include "weapon/Gun.h"
#include "speed/SpeedSystem.h"
#include "param/GameParam.h"
#include "state/StateMachine.h"
#include "actor/playerState/PlayerState.h"
#include <array>

class IController;

/** 待ち時間の種類 */
enum class EnCooldown
{
	WallRun,		///< 壁走り：離れてから、もう一度張り付けるまで
	Num				///< 種類の数（いちばん最後に置く）
};

/** 戦うキャラの土台。コマンド・待ち時間・能力を持つ（プレイヤーも敵も同じ） */
class BattleCharacter : public Character
{
public:
	BattleCharacter();

public:
	/** コマンドを作る係を設定する */
	void SetController(const IController* controller) { m_controller = controller; }
	/** 設定を渡す（NewGO の直後、Start より前に呼ぶ） */
	void SetParam(const CharacterParam& param) { m_param = &param; }
	/** 設定を返す（State も読む） */
	const CharacterParam& GetParam() const { return *m_param; }

public:
	/** このフレームのコマンドを返す（State や能力が読む） */
	const CharacterCommand& GetCommand() const { return m_command; }
	/** ジャンプが押されたか */
	bool IsJumpTrigger() const;
	/** スティックが倒れているか */
	bool IsMoveInput() const;
	/** スティックとカメラから決めた進行方向を返す */
	Vector3 GetMoveDirection() const;
	/** グラップルが押されたか */
	bool IsGrappleTrigger() const;
	/** グラップルを離したか */
	bool IsGrappleRelease() const;
	/** 射撃ボタンを押しているか */
	bool IsShootPress() const;
	/** エイムボタンを押しているか */
	bool IsAimPress() const;
	/** 待ち時間を始める（seconds 秒たつまで使えなくする） */
	void StartCooldown(const EnCooldown id, const float seconds);
	/** 待ち時間が終わったか */
	bool IsCooldownOver(const EnCooldown id) const;
	/** 速度を返す（HUD やデバッグ表示で読む） */
	const SpeedSystem& GetSpeedSystem() const { return m_speed; }
	/** 速度を返す（書き換えられる。デバッグ画面用） */
	SpeedSystem& GetSpeedSystem() { return m_speed; }
	/** グラップルの能力を返す（スイングの State がワイヤーに使う） */
	Grapple& GetGrapple() { return m_grapple; }
	/** スイングに使うロック中のアンカーを返す */
	Anchor* GetLockedAnchor() const { return m_grapple.GetLockedAnchor(); }
	/** スイングを始められるか（ロック中のアンカーがあるか） */
	bool CanStartSwing() const { return m_grapple.GetLockedAnchor() != nullptr; }
	/** 銃を返す（構えているかを見る・撃つ） */
	Gun& GetGun() { return m_gun; }

//stateが呼ぶ関数
public:
	/** スティックの向きに移動し、進む方向に体を向ける */
	void Move();
	/** 空中での移動。スティックの向きへ少しずつ曲がる（離すと勢いのまま） */
	void AirMove();
	/** 止まるまで減速する（Idle から呼ぶ） */
	void Decelerate();
	/** ジャンプする（上向きの速さを入れて、回数を1増やす） */
	void Jump();
	/** まだジャンプできるか（跳んだ回数が上限より少ないか） */
	bool CanJump() const { return m_jumpCount < m_param->jump.maxCount; }
	/** ジャンプの回数を 0 に戻す */
	void ResetJumpCount();
	/** ジャンプの回数を 1 増やす */
	void AddJumpCount();
	/** 地上のジャンプを使ったことにする（まだ跳んでいなければ回数を 1 にする） */
	void ConsumeGroundJump();
	/** 壁走りを始められるか */
	bool CanStartWallRun() const;
	/** 胸の位置を返す。ロープの手元と、銃口を決める基準に使う */
	Vector3 GetChestPosition() const;

public:
	/** 状態を切り替える */
	void ChangeState(const EnPlayerState id);
	/** 今の状態を返す（デバッグ画面の表示用） */
	EnPlayerState GetCurrentState() const { return m_stateMachine.GetCurrentState(); }

protected:
	/** 能力を準備する（子クラスの Start で呼ぶ） */
	void InitAbilities();
	/** ジャンプしたときに呼ばれる（子クラスがアニメーションなどを流す） */
	virtual void OnJump() {}
	/** 状態が切り替わったときに呼ばれる（子クラスがアニメーションなどを切り替える） */
	virtual void OnStateChanged(const EnPlayerState id) {}
	/** State を作って登録し、Idle から始める（子クラスの Start で呼ぶ）。仮: ステップ 6 で JSON の一覧から作る */
	void InitState();
	/** 1 フレームの行動。コマンド → ロック → State → 速さ → 構え → 射撃 の順に動かす（子クラスは上書きしない） */
	void UpdateAction() override final;
	/** State を動かした直後に呼ばれる（子クラスが体の傾きやアニメーションを合わせる） */
	virtual void OnStateUpdated() {}

private:
	/** コマンドを作り、待ち時間を進める */
	void UpdateCommand();
	/** 今の State を動かす */
	void UpdateState();
	/** 待ち時間を進める（0 より下にはしない） */
	void UpdateCooldown();
	/** 構えている間、体をカメラの前向きに向ける */
	void FaceAimDirection();
	/** 今の状態に合わせて、上乗せの速さを増やす・減らす（壁走りで増え、地上で減る） */
	void UpdateSpeedBonus();

private:
	static constexpr float MOVE_INPUT_THRESHOLD = 0.001f;						///< スティックを倒したとみなす境目（長さの2乗）
	static constexpr int COOLDOWN_NUM = static_cast<int>(EnCooldown::Num);		///< 待ち時間の種類の数

	const IController* m_controller = nullptr;									///< コマンドを作る係（Game が持っている）
	CharacterCommand m_command;													///< このフレームのコマンド
	std::array<float, COOLDOWN_NUM> m_cooldownTimers = {};						///< 待ち時間の残り（秒）
	Grapple m_grapple;															///< グラップルの能力（狙うアンカーの選択とワイヤーの見た目）
	Gun m_gun;																	///< 銃。RT で弾を撃つ
	SpeedSystem m_speed;														///< スピードシステム（基礎速度と上乗せ速度をもつ）
	const CharacterParam* m_param = nullptr;									///< このキャラの設定（nsGameParam が持っているものを借りる。SetParam で受け取る）
	const SpeedParam& m_speedParam = nsGameParam::Get().speed;					///< 速度の調整値（speed.json）
	int m_jumpCount = 0;														///< いま何回跳んだか
	StateMachine<BattleCharacter, EnPlayerState> m_stateMachine;				///< 状態の切り替え役
};
