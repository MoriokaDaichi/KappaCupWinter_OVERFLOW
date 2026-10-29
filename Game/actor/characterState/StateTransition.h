#pragma once
#include "actor/characterState/CharacterStateId.h"
#include "param/NameToEnum.h"
#include <string>
#include <vector>

/**
 * 切り替え表の部品。表そのものはキャラの JSON の transitions に書く
 * 「どの State のとき・何が成り立ったら・どこへ・切り替える前に何をするか」を 1 行で表す
 */

/** 切り替えの条件。判定の中身は C++（BattleCharacter::IsConditionMet）に書き、JSON には名前だけを書く */
enum class EnCondition : int {
    NotOnGround,    ///< 地面にいない（足場から落ちた）
    OnGround,       ///< 地面にいる
    Landed,         ///< 下に向かっていて地面にいる（跳んだ直後はまだ接地扱いなので、下向きのときだけ着地とみなす）
    MoveInput,      ///< スティックを倒している
    NoMoveInput,    ///< スティックを離している
    JumpTrigger,    ///< ジャンプを押した瞬間
    JumpStart,      ///< ジャンプを押した瞬間で、まだ跳べる（回数が残っている）
    GrappleStart,   ///< グラップルを押した瞬間で、ロック中のアンカーがある
    GrappleRelease, ///< グラップルを離した瞬間
    WallFound,      ///< 壁走りを始められる速さで、横に壁がある
    WallLost,       ///< 横に壁がなくなった
    Hooked,         ///< ワイヤーの先がアンカーに届いている（Swing だけが知っている）
    Num             ///< 個数
};

/** EnCondition の名前（JSON に書く名前）。enum と同じ順番に並べる */
constexpr const char* CONDITION_NAMES[] =
{
    "NotOnGround", "OnGround", "Landed", "MoveInput", "NoMoveInput", "JumpTrigger", "JumpStart",
    "GrappleStart", "GrappleRelease", "WallFound", "WallLost", "Hooked"
};

static_assert(sizeof(CONDITION_NAMES) / sizeof(CONDITION_NAMES[0]) == static_cast<int>(EnCondition::Num),
    "CONDITION_NAMES の数を EnCondition と合わせてください");

/**
 * その State でしか判定できない条件と、その State（それ以外は Num ＝どこでも使える）
 * 例えば Hooked は Swing の State が持つデータで判定するので、from が Swing の行にしか書けない
 */
constexpr EnCharacterState CONDITION_ONLY_FROM[] =
{
    EnCharacterState::Num, EnCharacterState::Num, EnCharacterState::Num, EnCharacterState::Num, EnCharacterState::Num,
    EnCharacterState::Num, EnCharacterState::Num, EnCharacterState::Num, EnCharacterState::Num, EnCharacterState::Num,
    EnCharacterState::Num, EnCharacterState::Swing
};

static_assert(sizeof(CONDITION_ONLY_FROM) / sizeof(CONDITION_ONLY_FROM[0]) == static_cast<int>(EnCondition::Num),
    "CONDITION_ONLY_FROM の数を EnCondition と合わせてください");

/** 切り替える前にやること。中身は C++ に書き、JSON には名前だけを書く */
enum class EnTransitionAction : int {
    Jump,           ///< ジャンプする（BattleCharacter::Jump）
    WallJump,       ///< 壁から跳ねる（WallRun の State が壁の向きを知っているので、State にやってもらう）
    ReleaseBoost,   ///< スイングを離したときの加速（Swing の State にやってもらう）
    Num             ///< 個数
};

/** EnTransitionAction の名前（JSON に書く名前）。enum と同じ順番に並べる */
constexpr const char* ACTION_NAMES[] =
{
    "Jump", "WallJump", "ReleaseBoost"
};

static_assert(sizeof(ACTION_NAMES) / sizeof(ACTION_NAMES[0]) == static_cast<int>(EnTransitionAction::Num),
    "ACTION_NAMES の数を EnTransitionAction と合わせてください");

/** その State でしかできない動作と、その State（それ以外は Num ＝どこでも使える） */
constexpr EnCharacterState ACTION_ONLY_FROM[] =
{
    EnCharacterState::Num, EnCharacterState::WallRun, EnCharacterState::Swing
};

static_assert(sizeof(ACTION_ONLY_FROM) / sizeof(ACTION_ONLY_FROM[0]) == static_cast<int>(EnTransitionAction::Num),
    "ACTION_ONLY_FROM の数を EnTransitionAction と合わせてください");

/** JSON の from に書くと「どの State からでも」になる名前 */
constexpr const char* ANY_STATE_NAME = "Any";

/** name に合う EnCondition を outId に入れる。知らない名前なら false */
inline bool ToCondition(const std::string& name, EnCondition& outId)
{
    return NameToEnum(CONDITION_NAMES, name, outId);
}

/** name に合う EnTransitionAction を outId に入れる。知らない名前なら false */
inline bool ToTransitionAction(const std::string& name, EnTransitionAction& outId)
{
    return NameToEnum(ACTION_NAMES, name, outId);
}

/** 切り替え表の 1 行（JSON の名前を enum に直したもの） */
struct StateTransition
{
    bool isFromAny = false;                                 ///< true ならどの State からでも（JSON の from が "Any"）
    EnCharacterState from = EnCharacterState::Num;          ///< どの State のときか（isFromAny のときは使わない）
    std::vector<EnCondition> when;                          ///< 条件。全部成り立ったら切り替える（左から順に調べ、だめなものがあればそこでやめる）
    EnCharacterState to = EnCharacterState::Num;            ///< 行き先
    std::vector<EnTransitionAction> actions;                ///< 切り替える前に、左から順にやること（なければ空）
};
