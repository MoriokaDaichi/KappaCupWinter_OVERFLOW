#pragma once
#include "param/GameParam.h"

/** 撃った弾。まっすぐ飛び、何かに当たるか時間が切れたら消える */
class Bullet : public IGameObject
{
public:
	bool Start() override;
	void Update() override;
	void Render(RenderContext& rc) override;

public:
	/** 出す位置と速度を決める。Gun が NewGO のすぐ後に呼ぶ */
	void Init(const Vector3& position, const Vector3& velocity);

private:
	ModelRender m_modelRender;								///< 見た目

	const GunParam& m_param = nsGameParam::Get().gun;		///< 調整用の値（Assets/param/gun.json）

	Vector3 m_position = Vector3::Zero;						///< 今の位置
	Vector3 m_velocity = Vector3::Zero;						///< 速度（向き × 速さ）。撃った後は変わらない
	float m_lifeTimer = 0.0f;								///< 消えるまでの残り時間（秒）
};
