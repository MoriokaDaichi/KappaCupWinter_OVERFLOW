#pragma once
#include "param/NameToEnum.h"
#include <string>

/** キャラクターの状態（プレイヤーも敵も同じ番号を使う。どれを持つかはキャラの JSON の states で決める） */
enum class EnCharacterState : int {
    Idle,   ///< 止まっている
    Run,    ///< 走っている
    Air,    ///< 空中（ジャンプ中・落下中）
    WallRun,///< 壁走り
    Swing,  ///< グラップルスイング
    Num     ///< 個数（配列の大きさに使う）
};

/** EnCharacterState の名前（JSON の states・transitions に書く名前）。enum と同じ順番に並べる */
constexpr const char* STATE_NAMES[] =
{
    "Idle", "Run", "Air", "WallRun", "Swing"
};

static_assert(sizeof(STATE_NAMES) / sizeof(STATE_NAMES[0]) == static_cast<int>(EnCharacterState::Num),
    "STATE_NAMES の数を EnCharacterState と合わせてください");

/** name に合う EnCharacterState を outId に入れる。知らない名前なら false */
inline bool ToCharacterState(const std::string& name, EnCharacterState& outId)
{
    return NameToEnum(STATE_NAMES, name, outId);
}
