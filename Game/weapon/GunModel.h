#pragma once

/** 手に持つ銃の見た目。毎フレーム、手の骨に合わせて置く（弾を撃つ処理は Gun）。BattleCharacter が持つ（銃と手の骨の両方があるときだけ） */
class GunModel
{
public:
	/** モデルを読み込む */
	void Init();
	/** 手の骨の行列（向き）と、握る位置（手のひら）に合わせる。骨が動いた後に呼ぶ */
	void Update(const Matrix& handBone, const Vector3& gripPosition);
	/** 描く */
	void Draw(RenderContext& rc);

private:
	ModelRender m_model;	///< 銃のモデル（原点はグリップの中央、銃口は体と同じ前向き）
};
