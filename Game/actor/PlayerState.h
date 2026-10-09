#pragma once

/** プレイヤーの状態 */
enum class EnPlayerState : int {
    Idle,   ///< 止まっている
    Run,    ///< 走っている
    Air,    ///< 空中（ジャンプ中・落下中）
    Num     ///< 個数（配列の大きさに使う）
};
