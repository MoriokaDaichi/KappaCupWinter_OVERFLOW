#include "stdafx.h"
#include "weapon/TurretModel.h"

void TurretModel::Init(const CharacterParam::Turret& param, const float scale, const Vector3& position)
{
	m_param = &param;
	m_scale = scale;

	// 体と同じく、倒されて消える・湧き直すのでレイトレの世界に入れない
	m_model.SetRaytracingWorld(false);
	m_model.Init(param.model.c_str());
	m_model.SetScale(Vector3(scale, scale, scale));

	Update(position, Vector3::Zero);
}

void TurretModel::Update(const Vector3& position, const Vector3& aimDirection)
{
	const Vector3 horizontal(aimDirection.x, 0.0f, aimDirection.z);
	const float horizontalLength = horizontal.Length();

	if (horizontalLength >= MIN_AIM_LENGTH)
	{
		// 砲口はモデルの -Z 向きなので、+Z を反対の向きに合わせる
		Quaternion yaw;
		yaw.SetRotationYFromDirectionXZ(Vector3(-horizontal.x, 0.0f, -horizontal.z));

		// 上下は台座から外れて見えないように maxPitch までにする
		const float maxPitch = Math::DegToRad(m_param->maxPitch);
		const float pitchAngle = max(-maxPitch, min(atan2f(aimDirection.y, horizontalLength), maxPitch));
		Quaternion pitch;
		pitch.SetRotationX(pitchAngle);

		m_rotation = pitch;
		m_rotation.Multiply(yaw);
	}

	m_pivot = position;
	m_pivot.y += (m_param->height + m_param->pivotHeight) * m_scale;

	// モデルの原点は底なので、回す中心が動かないように位置をずらす
	Vector3 pivotToOrigin(0.0f, -m_param->pivotHeight * m_scale, 0.0f);
	m_rotation.Apply(pivotToOrigin);

	m_model.SetRotation(m_rotation);
	m_model.SetPosition(m_pivot + pivotToOrigin);
	m_model.Update();
}

void TurretModel::Draw(RenderContext& rc)
{
	m_model.Draw(rc);
}

Vector3 TurretModel::GetMuzzlePosition() const
{
	Vector3 pivotToMuzzle(0.0f, (m_param->muzzleHeight - m_param->pivotHeight) * m_scale, -m_param->muzzleForward * m_scale);
	m_rotation.Apply(pivotToMuzzle);

	return m_pivot + pivotToMuzzle;
}
