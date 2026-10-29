#pragma once
#include "actor/characterState/CharacterStateId.h"
#include "param/GameParam.h"
#include "param/NameToEnum.h"
#include <array>
#include <string>

/** キャラクターのアニメーション（どのファイルを使うかはキャラの JSON の animation に書く） */
enum class EnCharacterAnim : int {
    Idle,     ///< 待機
    Run,      ///< 走る（壁走りでも使う。体を傾けて壁を走って見せる）
    JumpUp,   ///< 踏み切り〜最高点（ジャンプしたときに1回だけ流す）
    Fall,     ///< 空中にいる間（上へ向かっているときも。繰り返す。Falling Idle）
    FallFast, ///< 速く落ちている間（繰り返す。手足をばたつかせる Falling）
    Land,     ///< 止まって着地したとき（1回だけ流し、終わったら Idle。Falling To Landing の足がついてから後ろ）
    Swing,    ///< スイング
    Num       ///< 個数（配列の大きさに使う）
};

/** EnCharacterAnim の名前（JSON の animation.clips の anim に書く名前）。enum と同じ順番に並べる */
constexpr const char* ANIM_NAMES[] =
{
    "Idle", "Run", "JumpUp", "Fall", "FallFast", "Land", "Swing"
};

static_assert(sizeof(ANIM_NAMES) / sizeof(ANIM_NAMES[0]) == static_cast<int>(EnCharacterAnim::Num),
    "ANIM_NAMES の数を EnCharacterAnim と合わせてください");

/** name に合う EnCharacterAnim を outId に入れる。知らない名前なら false */
inline bool ToCharacterAnim(const std::string& name, EnCharacterAnim& outId)
{
    return NameToEnum(ANIM_NAMES, name, outId);
}

/**
 * キャラクターのアニメーションの切り替え役（もとは Player の中にあったもの）
 * 状態が変わったとき・ジャンプしたとき・毎フレームに、どのアニメーションを流すかを決める
 * JSON に animation を書いたキャラだけが持つ（BattleCharacter が作る）
 */
class CharacterAnimation : public Noncopyable
{
public:
	/** 設定のファイルを読み込む（ModelRender に渡すので、InitCharacter の前に呼ぶ） */
	void Load(const CharacterParam::Animation& param);
	/** 体のモデルを受け取る（InitCharacter の後、最初の状態に切り替える前に呼ぶ） */
	void SetModel(ModelRender& model) { m_model = &model; }
	/** ModelRender に渡すアニメーションの配列 */
	AnimationClip* GetClips() { return m_clips.data(); }
	/** ModelRender に渡すアニメーションの数 */
	int GetClipCount() const { return static_cast<int>(m_clips.size()); }

public:
	/** ジャンプしたとき。2段目のジャンプでも、踏み切りから流し直す */
	void OnJump();
	/** 状態が切り替わったとき。verticalSpeed は今の上下の速さ（空中のアニメーションを選ぶのに使う） */
	void OnStateChanged(const EnCharacterState state, const float verticalSpeed);
	/** 毎フレーム。空中にいる間は速さに合わせて切り替え、着地のアニメーションが終わったら待機につなぐ */
	void Update(const EnCharacterState state, const float verticalSpeed);

private:
	/** アニメーションを切り替える。同じアニメーションなら、isRestart が true のときだけ最初から流し直す */
	void Play(const EnCharacterAnim anim, const bool isRestart = false);
	/** 空中のアニメーションを選ぶ。踏み切りの途中ならそのまま、速く落ちていれば FallFast、それ以外は Fall */
	EnCharacterAnim SelectAirAnimation(const float verticalSpeed) const;

private:
	static constexpr float INTERPOLATE_TIME = 0.2f;	///< アニメーションを切り替えるときに、前のものとなめらかにつなぐ時間（秒）。調整は遷移のタスクで JSON に出す

	std::array<AnimationClip, static_cast<int>(EnCharacterAnim::Num)> m_clips;	///< アニメーション（ModelRender が借りるので、ここで持つ）
	const CharacterParam::Animation* m_param = nullptr;	///< 落下・着地の速さの境目（Load で受け取る）
	ModelRender* m_model = nullptr;					///< 体のモデル（借りているだけ）
	EnCharacterAnim m_currentAnim = EnCharacterAnim::Num;	///< 今流しているアニメーション（Num はまだ流していない）
	float m_airFallSpeed = 0.0f;					///< 空中で最後に見た、落ちる速さ（着地すると速さが 0 になるので、着地のアニメーションを流すかの判定用に覚えておく）
};
