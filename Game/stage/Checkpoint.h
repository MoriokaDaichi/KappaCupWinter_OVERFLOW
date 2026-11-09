#pragma once

class BattleCharacter;

/**
 * プレイヤーがどの区間まで進んだかを覚え、落ちたらその区間のチェックポイントへ戻す。Game が持つ
 */
class Checkpoint
{
public:
	/** 見張るプレイヤーを受け取る（Game の Start で呼ぶ） */
	void Init(BattleCharacter* player);
	/** 進んだ区間を覚えて、落ちていたら戻す */
	void Update();
	/** 今のチェックポイントの番号を返す */
	int GetIndex() const { return m_index; }

private:
	/** 今のチェックポイントへ戻し、上乗せを0にする */
	void Respawn();

private:
	static constexpr float FALL_HEIGHT = -1500.0f;		///< これより下に落ちたら戻す高さ

	BattleCharacter* m_player = nullptr;				///< 見張るプレイヤー（借りているだけなので、消さない）
	int m_index = 0;									///< 今のチェックポイントの番号（前にしか進めない）
};
