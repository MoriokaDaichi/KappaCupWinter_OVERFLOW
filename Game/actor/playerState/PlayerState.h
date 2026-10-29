#pragma once
#include <string>

/** プレイヤーの状態 */
enum class EnPlayerState : int {
    Idle,   ///< 止まっている
    Run,    ///< 走っている
    Air,    ///< 空中（ジャンプ中・落下中）
    WallRun,///< 壁走り
    Swing,  ///< グラップルスイング
    Num     ///< 個数（配列の大きさに使う）
};

/** プレイヤーのアニメーション（Assets/animData/player） */
enum class EnPlayerAnim : int {
    Idle,   ///< 待機
    Run,    ///< 走る（壁走りでも使う。体を傾けて壁を走って見せる）
    JumpUp,   ///< 踏み切り〜最高点（ジャンプしたときに1回だけ流す）
    Fall,     ///< 空中にいる間（上へ向かっているときも。繰り返す。Falling Idle）
    FallFast, ///< 速く落ちている間（繰り返す。手足をばたつかせる Falling）
    Land,     ///< 止まって着地したとき（1回だけ流し、終わったら Idle。Falling To Landing の足がついてから後ろ）
    Swing,  ///< スイング
    Num     ///< 個数（配列の大きさに使う）
};

/** EnPlayerState の名前（JSON の states に書く名前）。enum と同じ順番に並べる */
constexpr const char* PLAYER_STATE_NAMES[] =
{
    "Idle", "Run", "Air", "WallRun", "Swing"
};

static_assert(sizeof(PLAYER_STATE_NAMES) / sizeof(PLAYER_STATE_NAMES[0]) == static_cast<int>(EnPlayerState::Num),
    "PLAYER_STATE_NAMES の数を EnPlayerState と合わせてください");

/** name に合う EnPlayerState を outId に入れる。知らない名前なら false */
inline bool ToPlayerState(const std::string& name, EnPlayerState& outId)
{
    for (int i = 0; i < static_cast<int>(EnPlayerState::Num); i++)
    {
        if (name == PLAYER_STATE_NAMES[i])
        {
            outId = static_cast<EnPlayerState>(i);
            return true;
        }
    }
    return false;
}
