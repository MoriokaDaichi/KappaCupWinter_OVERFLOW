#pragma once
#include "actor/Character.h"
#include "actor/CharacterCommand.h"
#include <array>

class IController;

/** 待ち時間の種類 */
enum class EnCooldown
{
	WallRun,		///< 壁走り：離れてから、もう一度張り付けるまで
	Num				///< 種類の数（いちばん最後に置く）
};

/** 戦うキャラの土台。コマンドと待ち時間を持つ（プレイヤーも敵も同じ） */
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

protected:
	/** コマンドを作り、待ち時間を進める（子クラスの UpdateAction の最初に呼ぶ） */
	void UpdateCommand();

private:
	/** 待ち時間を進める（0 より下にはしない） */
	void UpdateCooldown();

private:
	static constexpr float MOVE_INPUT_THRESHOLD = 0.001f;						///< スティックを倒したとみなす境目（長さの2乗）
	static constexpr int COOLDOWN_NUM = static_cast<int>(EnCooldown::Num);		///< 待ち時間の種類の数
	const IController* m_controller = nullptr;									///< コマンドを作る係（Game が持っている）
	CharacterCommand m_command;													///< このフレームのコマンド
	std::array<float, COOLDOWN_NUM> m_cooldownTimers = {};						///< 待ち時間の残り（秒）
};
