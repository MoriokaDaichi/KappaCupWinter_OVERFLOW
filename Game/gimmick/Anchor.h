#pragma once
#include "param/GameParam.h"

/** グラップルで狙える固定のアンカー。位置と見た目だけを持つ */
class Anchor : public IGameObject
{
public:
	bool Start() override;
	void Update() override;
	void Render(RenderContext& rc) override;

public:
	/** 置く場所を決める。Game が NewGO のすぐ後に呼ぶ */
	void SetPosition(const Vector3& pos) { m_position = pos; }
	/** 位置を返す。Player がロックの判定やスイングに使う */
	const Vector3& GetPosition() const { return m_position; }
	/** ロック中かどうかを受け取る。見た目の大きさに使う */
	void SetLocked(const bool locked) { m_isLocked = locked; }

private:
	const AnchorParam& m_param = nsGameParam::Get().anchor;	///< 調整用の値（Assets/param/anchor.json）

	ModelRender m_modelRender;						///< 見た目
	Vector3 m_position = Vector3::Zero;				///< 位置
	bool m_isLocked = false;						///< ロック中か
};
