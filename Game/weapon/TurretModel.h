#pragma once
#include "param/GameParam.h"

/**
 * 台座の上に載せる砲身の見た目。毎フレーム、狙いの向きへ回す（弾を撃つ処理は EnemySpawner）
 * BattleCharacter が持つ（設定に turret があるときだけ）
 */
class TurretModel
{
public:
	/** モデルを読み込む。scale は見た目の倍率（設定の長さにも掛ける）、position は足元の位置 */
	void Init(const CharacterParam::Turret& param, const float scale, const Vector3& position);
	/** position（足元の位置）の上に置き、aimDirection の方へ向ける。向きがないとき（長さ 0）は今の向きのまま */
	void Update(const Vector3& position, const Vector3& aimDirection);
	/** 描く */
	void Draw(RenderContext& rc);
	/** 砲口の位置（2 本の砲身の間）を返す */
	Vector3 GetMuzzlePosition() const;

private:
	static constexpr float MIN_AIM_LENGTH = 0.001f;		///< 横の向きを決められる長さの下限（真上・真下を狙うときは横の向きを変えない）

	ModelRender m_model;								///< 砲身のモデル（原点は底の中央、砲口は -Z 向き）
	const CharacterParam::Turret* m_param = nullptr;	///< 砲身の設定（nsGameParam が持っているものを借りる）
	float m_scale = 1.0f;								///< 見た目の倍率
	Vector3 m_pivot;									///< 回す中心の位置
	Quaternion m_rotation;								///< 今の向き
};
