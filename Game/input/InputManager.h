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
    /** 複製できないようにする（入力は 1 つだけ） */
    InputManager(const InputManager&) = delete;
    InputManager& operator = (const InputManager&) = delete;
    void Update();

public:
    /**押した瞬間*/
    bool IsTrigger(const EnInputAction a) const;
    /**押している間*/
    bool IsPress(const EnInputAction a) const;
    /**離した瞬間*/
    bool IsRelease(const EnInputAction a) const;
    /**押し続けている秒数*/
    float GetHoldTime(const EnInputAction a) const;
    /**左スティック*/
    Vector2 GetMove() const;
    /**右スティック*/
    Vector2 GetCamera() const;
    /**入力を止める・戻す*/
    void SetEnable(const bool enable);
    /** 唯一の実体を返す（受け取るときは InputManager& で受ける） */
    static InputManager& GetInstance();

private:
    InputManager() = default;
 
private:
    static constexpr int ACTION_NUM = static_cast<int>(EnInputAction::Num);

    bool m_prevPress[ACTION_NUM] = {};
    bool m_press[ACTION_NUM] = {};
    float m_holdTime[ACTION_NUM] = {};
    bool m_enable = true;

    /** 操作を配列の添え字に変える */
    static int ToIndex(const EnInputAction a);
};

