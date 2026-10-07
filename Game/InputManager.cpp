#include "stdafx.h"
#include "InputManager.h"

namespace
{
    /** EnInputAction の順番に、対応するパッドのボタンを並べる */
    const EnButton ACTION_TO_BUTTON[] = {
        enButtonA,    // Jump
        enButtonRB2,  // Shoot
        enButtonLB2,  // Aim
        enButtonRB1,  // Grapple
        enButtonStart // Pause
    };

    static_assert(std::size(ACTION_TO_BUTTON) == static_cast<size_t>(EnInputAction::Num),
        "ACTION_TO_BUTTON の要素数が EnInputAction と合っていません");
}

void InputManager::Update()
{
    if (!m_enable)
    {
        for (int i = 0; i < ACTION_NUM; i++)
        {
            m_prevPress[i] = false;
            m_press[i] = false;
            m_holdTime[i] = 0.0f;
        }
        return;
    }

    for (int i = 0; i < ACTION_NUM; i++)
    {
        m_prevPress[i] = m_press[i];
        m_press[i] = g_pad[0]->IsPress(ACTION_TO_BUTTON[i]);

        if (m_press[i])
        {
            m_holdTime[i] += g_gameTime->GetFrameDeltaTime();
        }
        else
        {
            m_holdTime[i] = 0.0f;
        }
    }
}

bool InputManager::IsTrigger(EnInputAction a) const
{
    return m_press[ToIndex(a)] && !m_prevPress[ToIndex(a)];
}

bool InputManager::IsPress(EnInputAction a) const
{
    return m_press[ToIndex(a)];
}

bool InputManager::IsRelease(EnInputAction a) const
{
    return !m_press[ToIndex(a)] && m_prevPress[ToIndex(a)];
}

float InputManager::GetHoldTime(EnInputAction a) const
{
    return m_holdTime[ToIndex(a)];
}

Vector2 InputManager::GetMove() const
{
    if (!m_enable)     //止めているなら
    {
        return Vector2::Zero;
    }
    return Vector2(g_pad[0]->GetLStickXF(), g_pad[0]->GetLStickYF());
}

Vector2 InputManager::GetCamera() const
{
    if (!m_enable)    //止めているなら
    {
        return Vector2::Zero;
    }
    return Vector2(g_pad[0]->GetRStickXF(), g_pad[0]->GetRStickYF());
}

void InputManager::SetEnable(bool enable)
{
    m_enable = enable;
}

int InputManager::ToIndex(EnInputAction a)
{
    const int i = static_cast<int>(a);
    K2_ASSERT(i >= 0 && i < ACTION_NUM, "EnInputAction の範囲外です");
    return i;
}
