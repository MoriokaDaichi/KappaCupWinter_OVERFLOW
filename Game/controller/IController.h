#pragma once
#include "actor/CharacterCommand.h"

/** キャラクターのコマンドを作る係の親 */
class IController
{
public:
	virtual ~IController() = default;
	/** コマンドを作る。中身は子のクラスが書く */
	virtual void MakeCommand(CharacterCommand& command) const = 0;
};
