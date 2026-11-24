#include "stdafx.h"
#include "scene/MenuSelector.h"

#include "input/InputManager.h"

void MenuSelector::Reset(const int index)
{
	m_index = index;
	// 開いた瞬間にスティックを倒していても、すぐには動かさない
	m_lastStickDirection = 0;
}

bool MenuSelector::Update(const InputManager& input, const int count)
{
	if (count <= 0)
	{
		return false;
	}

	// スティックは上がプラスなので、上を -1（番号が減る向き）にそろえる
	const float stickY = input.GetMove().y;
	int stickDirection = 0;
	if (stickY > STICK_THRESHOLD)
	{
		stickDirection = -1;
	}
	else if (stickY < -STICK_THRESHOLD)
	{
		stickDirection = 1;
	}
	const bool isStickMoved = stickDirection != 0 && stickDirection != m_lastStickDirection;
	m_lastStickDirection = stickDirection;

	int move = 0;
	if (input.IsTrigger(EnInputAction::MenuUp) || (isStickMoved && stickDirection < 0))
	{
		move = -1;
	}
	else if (input.IsTrigger(EnInputAction::MenuDown) || (isStickMoved && stickDirection > 0))
	{
		move = 1;
	}

	if (move == 0)
	{
		return false;
	}

	// 端まで行ったら反対の端へ回る（+ count してから割るのは、-1 のときもマイナスにしないため）
	m_index = (m_index + move + count) % count;
	return true;
}
