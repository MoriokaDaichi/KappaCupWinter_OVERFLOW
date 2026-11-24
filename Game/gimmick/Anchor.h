#pragma once
#include "param/GameParam.h"

class BattleCharacter;

/**
 * グラップルで狙えるアンカー。位置と見た目だけを持つ
 * ステージに置く固定のものと、敵（CRAWLER）が体に付けて動かすものがある。どちらも持ち主は知らない
 * 敵のアンカーは、離されたことを覚えておき、持ち主の敵が TakeReleasedBy で受け取って倒れる
 */
class Anchor : public IGameObject
{
public:
	bool Start() override;
	void Update() override;
	void Render(RenderContext& rc) override;

public:
	/** 置く場所を決める。Stage が NewGO のすぐ後と JSON を読み直したとき、敵のアンカーは持ち主が毎フレーム呼ぶ */
	void SetPosition(const Vector3& pos) { m_position = pos; }
	/** 位置を返す。キャラ（AnchorLocker・SwingState）がロックの判定やスイングに使う */
	const Vector3& GetPosition() const { return m_position; }
	/** ロック中かどうかを受け取る。見た目の大きさに使う */
	void SetLocked(const bool locked) { m_isLocked = locked; }
	/** ぶら下がっていたキャラ（by）が離したことを覚える（SwingState が呼ぶ） */
	void NotifyReleased(BattleCharacter& by);
	/**
	 * 離したキャラを返し、覚えていたことを消す。離されていなければ nullptr
	 * 敵のアンカーの持ち主が毎フレーム呼ぶ（固定のアンカーは誰も呼ばないので、覚えたままでよい）
	 */
	BattleCharacter* TakeReleasedBy();

private:
	const AnchorParam& m_param = nsGameParam::Get().anchor;	///< 調整用の値（Assets/param/anchor.json）

	ModelRender m_modelRender;						///< 見た目
	Vector3 m_position = Vector3::Zero;				///< 位置
	bool m_isLocked = false;						///< ロック中か
	GOHandle<BattleCharacter> m_releasedBy;			///< 離したキャラ（まだ受け取られていなければ）。プレイヤーがやり直しで作り直されることもあるので GOHandle
};
