#pragma once
#include "controller/IController.h"

class InputManager;

/** プレイヤー用のコマンドを作る。入力に関わることは、全てこのクラスに書く */
class PlayerController : public IController
{
public:
	/** 入力とカメラから、このフレームのコマンドを作る */
	void MakeCommand(CharacterCommand& command) const override;
	/** 入力を受け取る先を設定する。Game が持っている InputManager を渡す */
	void SetInput(const InputManager* input)
	{
		m_input = input;
	}

private:
	/** スティックとカメラから進む方向を出す */
	Vector3 CalcMoveDirection() const;

private:
	static constexpr float MOVE_INPUT_THRESHOLD = 0.001f;	///< スティックを倒したとみなす境目（長さの2乗）

	const InputManager* m_input = nullptr;					///< 入力（読むだけ。止めるのは Game の役目）
};
