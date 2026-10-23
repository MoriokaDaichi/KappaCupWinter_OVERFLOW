#include "stdafx.h"
#include "weapon/GunModel.h"

namespace
{
	/**
	 * 手の骨の向きから、銃の向きへ直す回転。
	 * shoot.tka（両手で構える姿勢）のとき、銃口が体の前を向くように、手の骨の行列から求めた値（2026-10-23）。
	 * 主人公のモデルや射撃のアニメーションを変えたら測り直す
	 */
	const Quaternion HAND_TO_GUN(-0.3890f, -0.4223f, -0.5231f, 0.6299f);
}

void GunModel::Init()
{
	m_model.Init("Assets/modelData/player/gun.tkm");
}

void GunModel::Update(const Matrix& handBone, const Vector3& gripPosition)
{
	// 骨の行列から向きだけを取り出し、手の中での銃の向き（HAND_TO_GUN）を先にかける
	Quaternion handRotation;
	handRotation.SetRotation(handBone);
	Quaternion rotation = HAND_TO_GUN;
	rotation.Multiply(handRotation);

	m_model.SetRotation(rotation);
	m_model.SetPosition(gripPosition);
	m_model.Update();
}

void GunModel::Draw(RenderContext& rc)
{
	m_model.Draw(rc);
}
