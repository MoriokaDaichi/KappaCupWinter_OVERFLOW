#pragma once

class InputManager;

/**
 * メニューの「今どれを選んでいるか」。十字キーの上下と、左スティックの上下で動かす（端まで行くと反対の端へ回る）
 * スティックは倒した瞬間だけ 1 つ動かす（倒しっぱなしで毎フレーム動かないように）
 */
class MenuSelector
{
public:
	/** 選んでいるものを index にする（メニューを開いたとき） */
	void Reset(const int index);
	/** 入力を見て、count 個の中で選ぶものを動かす。動いたら true */
	bool Update(const InputManager& input, const int count);
	/** 選んでいるものの番号（上から 0, 1, 2…） */
	int GetIndex() const { return m_index; }

private:
	static constexpr float STICK_THRESHOLD = 0.5f;	///< スティックをこれより倒したら、倒したとみなす

	int m_index = 0;					///< 選んでいるものの番号
	int m_lastStickDirection = 0;		///< 前のフレームのスティックの向き（上 -1、下 +1、倒していない 0）
};
