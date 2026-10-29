#pragma once
#include "actor/BattleCharacter.h"
#include "weapon/GunModel.h"
#include <array>

class Anchor;

/** プレイヤー。移動・ジャンプ・射撃などを行う */
class Player : public BattleCharacter
{
public:
	bool Start();

public:
	/** position へ瞬間移動する。速さを 0 にして空中の状態から始める（デバッグ画面・チェックポイント用） */
	void Warp(const Vector3& position);

protected:
	/** 体を動かした後（骨の位置が決まった後）に、ワイヤーと銃の見た目を合わせる */
	void OnBodyUpdated() override;
	/** 体と、手に持つ銃を描く */
	void Render(RenderContext& rc) override;
	/** ジャンプしたときに呼ばれる（アニメーションなどを流す） */
	void OnJump() override;
	/** 状態が切り替わったときに呼ばれる（アニメーションなどを切り替える） */
	void OnStateChanged(const EnPlayerState id) override;
	/** State を動かした直後に、体の傾きとアニメーションを合わせる */
	void OnStateUpdated() override;

private:
	/** アニメーションを読み込む（InitCharacter の前に呼ぶ） */
	void LoadAnimation();
	/** 状態に合ったアニメーションに切り替える */
	void PlayStateAnimation(const EnPlayerState state);
	/** アニメーションを切り替える。同じアニメーションなら、isRestart が true のときだけ最初から流し直す */
	void PlayAnimation(const EnPlayerAnim anim, const bool isRestart = false);
	/** 空中にいる間は速さに合わせて切り替え、着地のアニメーションが終わったら待機につなぐ */
	void UpdateAnimation();
	/** 空中のアニメーションを選ぶ。踏み切りの途中ならそのまま、速く落ちていれば FallFast、それ以外は Fall */
	EnPlayerAnim SelectAirAnimation() const;
	/** ロープを持つ手の位置（両手の間）。骨が見つからないときは胸の位置 */
	Vector3 GetRopeHandPosition() const;
	/** 体の見た目を傾ける。スイングではアンカーの方へ、壁走りでは壁から離れる向きへ。ほかの状態ではまっすぐに戻す */
	void UpdateBodyTilt();
	/** ワイヤーの手元を渡す（骨が動いた後に呼ぶ） */
	void UpdateWireHand();
	/** 銃の見た目を右手に持たせる（握る位置は、手首と中指の付け根の間＝手のひら） */
	void UpdateGunModel();

private:
	GunModel m_gunModel;					///< 右手に持つ銃の見た目
	std::array<AnimationClip, static_cast<int>(EnPlayerAnim::Num)> m_animClips;	///< アニメーション（ModelRender が借りるので、ここで持つ）
	EnPlayerAnim m_currentAnim = EnPlayerAnim::Num;	///< 今流しているアニメーション（Num はまだ流していない）
	float m_airFallSpeed = 0.0f;			///< 空中で最後に見た、落ちる速さ（着地すると速さが 0 になるので、着地のアニメーションを流すかの判定用に覚えておく）
	int m_leftHandBoneId = -1;				///< 左手の骨の番号（ロープの手元に使う。見つからなければ -1）
	int m_rightHandBoneId = -1;				///< 右手の骨の番号
	int m_rightMiddleBoneId = -1;			///< 右手の中指の付け根の骨の番号（銃を握る位置に使う）

	// 調整する値は Assets/param/chara/player.json（意味は param/GameParam.h の CharacterParam）。ここには計算のための値だけ置く
	static constexpr float ANIM_INTERPOLATE_TIME = 0.2f;	///< アニメーションを切り替えるときに、前のものとなめらかにつなぐ時間（秒）。調整は遷移のタスクで JSON に出す
	static constexpr float START_HEIGHT = 300.0f;			///< 始めるときに置く高さ（床の少し上から落とす）
};
