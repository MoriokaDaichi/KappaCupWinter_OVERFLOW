#include "stdafx.h"
#include "actor/Enemy.h"

bool Enemy::Start()
{
	// アニメーションはまだないので、立ったまま動く
	InitCharacter("Assets/modelData/enemy/aegis.tkm", nullptr, 0,
		m_param.body.radius, m_param.body.height, m_startPosition);

	InitState();

	InitAbilities();

	return true;
}
