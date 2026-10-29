#include "stdafx.h"
#include "actor/Enemy.h"

bool Enemy::Start()
{
	// アニメーションはまだないので、立ったまま動く
	InitCharacter(GetParam().model.c_str(), nullptr, 0,
		GetParam().body.radius, GetParam().body.height, m_startPosition);

	InitState();

	InitAbilities();

	return true;
}
