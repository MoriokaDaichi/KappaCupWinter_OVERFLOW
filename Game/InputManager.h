#pragma once

enum class EnInputAction : int {
    Jump,     ///< A
    Shoot,    ///< RT
    Aim,      ///< LT
    Grapple,  ///< RB
    Pause,    ///< Start
    Num       ///< 個数（配列の大きさに使う）
};

class InputManager
{
public:
    void Update();

public:
    /**押した瞬間*/
    bool IsTrigger(EnInputAction a) const;
    /**押している間*/
    bool IsPress(EnInputAction a) const;
    /**離した瞬間*/
    bool IsRelease(EnInputAction a) const;
    /**押し続けている秒数*/
    float GetHoldTime(EnInputAction a) const;

public:
    /**左スティック*/
    Vector2 GetMove() const;
    /**右スティック*/
    Vector2 GetCamera() const;

public:
    /**入力を止める・戻す*/
    void SetEnable(bool enable);

private:
    static constexpr int ACTION_NUM = static_cast<int>(EnInputAction::Num);

    bool m_prevPress[ACTION_NUM] = {};
    bool m_press[ACTION_NUM] = {};
    float m_holdTime[ACTION_NUM] = {};
    bool m_enable = true;

    /** 操作を配列の添え字に変える */
    static int ToIndex(EnInputAction a);
};

