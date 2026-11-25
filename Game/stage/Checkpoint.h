#pragma once

class BattleCharacter;

/**
 * プレイヤーがどの区間まで進んだかを覚え、落ちたらその区間のチェックポイントへ戻す。Game が持つ
 * 落ちたらすぐには戻さず、SIGNAL_LOST_TIME 秒のあいだ「SIGNAL LOST」を見せてから戻す（企画書の失敗の絵。HUD が IsSignalLost を見て出す）
 */
class Checkpoint
{
public:
	/** 見張るプレイヤーを受け取る（Game の Start で呼ぶ） */
	void Init(BattleCharacter* player);
	/** 進んだ区間を覚えて、落ちていたら SIGNAL LOST を始め、時間が来たら戻す */
	void Update();
	/** 今のチェックポイントの番号を返す */
	int GetIndex() const { return m_index; }
	/** 最初の区間に戻す（最初からやり直すとき） */
	void Reset() { m_index = 0; m_signalLostTimer = 0.0f; }
	/** 今のチェックポイントへ戻し、上乗せを0にする（落ちたときと、ポーズの「チェックポイントから」） */
	void Respawn();
	/** 落ちて、戻るのを待っている間か（HUD が「SIGNAL LOST」を出す） */
	bool IsSignalLost() const { return m_signalLostTimer > 0.0f; }
	/** SIGNAL LOST が出てからの割合（0 で出た瞬間、1 で戻る瞬間） */
	float GetSignalLostRate() const { return 1.0f - m_signalLostTimer / SIGNAL_LOST_TIME; }

private:
	static constexpr float FALL_HEIGHT = -1500.0f;		///< これより下に落ちたら戻す高さ
	static constexpr float SIGNAL_LOST_TIME = 1.0f;		///< 落ちてから戻すまでの秒数（SIGNAL LOST を見せる時間）

	BattleCharacter* m_player = nullptr;				///< 見張るプレイヤー（借りているだけなので、消さない）
	int m_index = 0;									///< 今のチェックポイントの番号（前にしか進めない）
	float m_signalLostTimer = 0.0f;						///< 戻すまでの残り時間（秒）。0 なら落ちていない
};
