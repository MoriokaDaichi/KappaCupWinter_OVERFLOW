#pragma once
#include "actor/Character.h"
#include "actor/CharacterCommand.h"
#include "grapple/Grapple.h"
#include "weapon/Gun.h"
#include "speed/SpeedSystem.h"
#include "param/GameParam.h"
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
	/** コマンドを作る係を設定する */
	void SetController(const IController* controller) { m_controller = controller; }
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
	bool CanJump() const { return m_jumpCount < m_param.jump.maxCount; }
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

protected:
	/** コマンドを作り、待ち時間を進める（子クラスの UpdateAction の最初に呼ぶ） */
	void UpdateCommand();
	/** 能力を準備する（子クラスの Start で呼ぶ） */
	void InitAbilities();
	/** ジャンプしたときに呼ばれる（子クラスがアニメーションなどを流す） */
	virtual void OnJump() {}

private:
	/** 待ち時間を進める（0 より下にはしない） */
	void UpdateCooldown();

private:
	static constexpr float MOVE_INPUT_THRESHOLD = 0.001f;						///< スティックを倒したとみなす境目（長さの2乗）
	static constexpr int COOLDOWN_NUM = static_cast<int>(EnCooldown::Num);		///< 待ち時間の種類の数

	const IController* m_controller = nullptr;									///< コマンドを作る係（Game が持っている）
	CharacterCommand m_command;													///< このフレームのコマンド
	std::array<float, COOLDOWN_NUM> m_cooldownTimers = {};						///< 待ち時間の残り（秒）
	Grapple m_grapple;															///< グラップルの能力（狙うアンカーの選択とワイヤーの見た目）
	Gun m_gun;																	///< 銃。RT で弾を撃つ
	SpeedSystem m_speed;														///< スピードシステム（基礎速度と上乗せ速度をもつ）
	const PlayerParam& m_param = nsGameParam::Get().player;						///< 調整用の値。仮: ステップ 6 でキャラごとの設定から読む
	int m_jumpCount = 0;														///< いま何回跳んだか
};
