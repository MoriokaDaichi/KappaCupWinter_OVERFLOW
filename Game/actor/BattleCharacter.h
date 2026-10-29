#pragma once
#include "actor/Character.h"
#include "actor/CharacterCommand.h"
#include "actor/CharacterAnimation.h"
#include "actor/HandBones.h"
#include "actor/characterState/CharacterState.h"
#include "actor/characterState/CharacterStateId.h"
#include "actor/characterState/TransitionTable.h"
#include "grapple/Grapple.h"
#include "weapon/Gun.h"
#include "weapon/GunModel.h"
#include "speed/SpeedSystem.h"
#include "param/GameParam.h"
#include "state/StateMachine.h"
#include <array>
#include <memory>
#include <vector>

class IController;

/** 待ち時間の種類 */
enum class EnCooldown
{
	WallRun,		///< 壁走り：離れてから、もう一度張り付けるまで
	Num				///< 種類の数（いちばん最後に置く）
};

/**
 * 戦うキャラ。プレイヤーも敵もこのクラス 1 つで作る（Player・Enemy という子クラスはない）
 * 違いは 2 つだけ: 渡す設定（キャラの JSON）と、渡す Controller（PlayerController か AIController か）
 * このクラスがするのは「部品を持つこと」と「毎フレームの呼ぶ順番」だけ。動きの中身は State と能力に書く
 */
class BattleCharacter : public Character
{
public:
	BattleCharacter();
	/** 設定に書いた部品（アニメーション・手の骨・State・能力・銃の見た目）を作る */
	bool Start() override;

public:
	/** コマンドを作る係を設定する */
	void SetController(const IController* controller) { m_controller = controller; }
	/** 設定を渡す（NewGO の直後、Start より前に呼ぶ） */
	void SetParam(const CharacterParam& param) { m_param = &param; }
	/** 出てくる位置（足元）を決める（Start より前に呼ぶ） */
	void SetStartPosition(const Vector3& position) { m_startPosition = position; }
	/** 設定を返す（State も読む） */
	const CharacterParam& GetParam() const { return *m_param; }
	/** position へ瞬間移動する。速さを 0 にして空中の状態から始める（デバッグ画面・チェックポイント用） */
	void Warp(const Vector3& position);

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
	/** グラップルを持っているか */
	bool HasGrapple() const { return m_grapple != nullptr; }
	/** グラップルの能力を返す（持っているときだけ呼ぶ） */
	Grapple& GetGrapple() { return *m_grapple; }
	/** ロック中のアンカーを返す。グラップルがない・候補がないときは nullptr */
	Anchor* GetLockedAnchor() const { return m_grapple ? m_grapple->GetLockedAnchor() : nullptr; }
	/** スイングを始められるか */
	bool CanStartSwing() const { return GetLockedAnchor() != nullptr; }
	/** 銃を持っているか */
	bool HasGun() const { return m_gun != nullptr; }
	/** 銃を返す（持っているときだけ呼ぶ） */
	Gun& GetGun() { return *m_gun; }

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
	/** 状態を切り替える（ふつうは切り替え表が呼ぶ。ワープのように表の外から変えたいときだけ直接呼ぶ） */
	void ChangeState(const EnCharacterState id);
	/** 今の状態を返す（デバッグ画面の表示用） */
	EnCharacterState GetCurrentState() const { return m_stateMachine.GetCurrentState(); }
	/** id の状態を持っているか（設定の states に書いてあるか） */
	bool HasState(const EnCharacterState id) const { return m_stateMachine.HasState(id); }

protected:
	/** 1 フレームの行動。コマンド → ロック → 切り替え表と State → 体の傾き・アニメーション → 速さ → 構え → 射撃 の順に動かす */
	void UpdateAction() override final;
	/** 体を動かした後（骨の位置が決まった後）に、ワイヤーの手元と銃の見た目を手に合わせる */
	void OnBodyUpdated() override;
	/** 体と、手に持つ銃を描く */
	void Render(RenderContext& rc) override;

private:
	/** 設定の states に書いた State だけを作って登録し、切り替え表を作って、Idle から始める */
	void InitState();
	/** 設定の abilities に書いた能力を作る */
	void InitAbilities();
	/** コマンドを作り、待ち時間を進める */
	void UpdateCommand();
	/** 切り替え表を見て、必要なら State を切り替える。切り替えなかったら、今の State を動かす */
	void UpdateState();
	/** 待ち時間を進める（0 より下にはしない） */
	void UpdateCooldown();
	/** 体の見た目の傾きを、State が決めた目標へ少しずつ近づける（目標を出さなかったらまっすぐに戻る） */
	void UpdateModelTilt();
	/** 構えている間、体をカメラの前向きに向ける */
	void FaceAimDirection();
	/** 今の状態に合わせて、上乗せの速さを増やす・減らす（壁走りで増え、地上で減る） */
	void UpdateSpeedBonus();
	/** 設定の abilities に name があるか */
	bool HasAbility(const char* name) const;
	/** id に合う State を作って登録する（名前から State を作る工場） */
	void AddStateById(const EnCharacterState id);

private:
	static constexpr float MOVE_INPUT_THRESHOLD = 0.001f;						///< スティックを倒したとみなす境目（長さの2乗）
	static constexpr int COOLDOWN_NUM = static_cast<int>(EnCooldown::Num);		///< 待ち時間の種類の数

	const IController* m_controller = nullptr;									///< コマンドを作る係（Game が持っている）
	CharacterCommand m_command;													///< このフレームのコマンド
	std::array<float, COOLDOWN_NUM> m_cooldownTimers = {};						///< 待ち時間の残り（秒）
	SpeedSystem m_speed;														///< スピードシステム（基礎速度と上乗せ速度をもつ）
	const CharacterParam* m_param = nullptr;									///< このキャラの設定（nsGameParam が持っているものを借りる。SetParam で受け取る）
	const SpeedParam& m_speedParam = nsGameParam::Get().speed;					///< 速度の調整値（speed.json）
	Vector3 m_startPosition = Vector3::Zero;									///< 出てくる位置（足元）
	int m_jumpCount = 0;														///< いま何回跳んだか
	StateMachine<BattleCharacter, EnCharacterState, CharacterState> m_stateMachine;	///< 状態の切り替え役
	TransitionTable m_transitionTable;											///< 切り替え表（設定の transitions から作る）
	std::unique_ptr<Grapple> m_grapple;											///< グラップルの能力（abilities に "Grapple" があるときだけ作る）
	std::unique_ptr<Gun> m_gun;													///< 銃（abilities に "Gun" があるときだけ作る）
	std::unique_ptr<CharacterAnimation> m_animation;							///< アニメーション（設定に animation があるときだけ作る）
	std::unique_ptr<HandBones> m_handBones;										///< 手の骨（設定に hands があるときだけ作る）
	std::unique_ptr<GunModel> m_gunModel;										///< 手に持つ銃の見た目（銃と手の骨の両方があるときだけ作る）
};
