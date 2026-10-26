#pragma once
#include "param/GameParam.h"

/** 撃った弾をまとめて動かし、まとめて描く（１発ごとにInitすると重いため）。Game が NewGO する */
class BulletManager : public IGameObject
{
public:
	bool Start() override;
	void Update() override;
	void Render(RenderContext& rc) override;

public:
	/** 空いている枠を探して弾を出す。空きがなければ撃たない */
	void Fire(const Vector3& position, const Vector3& velocity, const GunParam::Level& levelParam);

private:
	/** 弾を消して空きに戻す（描かれないように大きさ 0 にする） */
	void Deactivate(const int index);

private:
	/** 弾1発分 */
	struct Bullet
	{
		bool isActive = false;		///< 飛んでいるか（false なら空き）
		Vector3 position;			///< 今の位置
		Vector3 velocity;			///< 速度
		float lifeTimer = 0.0f;		///< 消えるまでの残り時間
		float scale = 0.0f;			///< 大きさ
		bool pierce = false;		///< 弾が貫通するかどうか
		float explosionRadius = 0.0f;	///< 爆発の半径（0 なら爆発しない）
	};

	static constexpr int MAX_BULLET = 128;					///< 同時に飛べる弾の数（連射 0.08 秒 × 寿命 2 秒 ≒ 25 発に余裕を持たせる）
	std::array<Bullet, MAX_BULLET> m_bullets;				///< 全部の弾。isActive が false の枠は空き
	ModelRender m_modelRender;								///< 弾の見た目（全部の弾でこれ1つ）
	const GunParam& m_param = nsGameParam::Get().gun;		///< 調整用の値（Assets/param/gun.json）
};
