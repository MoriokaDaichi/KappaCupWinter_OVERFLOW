#pragma once

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
	void SetLocked(bool locked) { m_isLocked = locked; }

private:
	static constexpr float NORMAL_SCALE = 1.0f;		///< ふだんの大きさ（直径100）
	static constexpr float LOCKED_SCALE = 1.5f;		///< ロック中の大きさ

	ModelRender m_modelRender;						///< 見た目
	Vector3 m_position = Vector3::Zero;				///< 位置
	bool m_isLocked = false;						///< ロック中か
};
