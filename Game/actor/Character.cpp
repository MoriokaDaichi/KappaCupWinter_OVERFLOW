#include "stdafx.h"
#include "actor/Character.h"

void Character::Update()
{
	if (m_hitStopTimer > 0.0f)
	{
		// 止まっている間も、体に付いているもの（輪郭線・アンカー）は見た目に合わせる
		UpdateHitStop();
		OnBodyUpdated();
		return;
	}

	UpdateAction();
	UpdateBody();
	OnBodyUpdated();
}

void Character::StartHitStop(const float seconds, const float amplitude)
{
	if (seconds <= 0.0f)
	{
		return;
	}

	// 止まっている途中にまた当たったら、震えの中心は最初に止まった位置のまま、時間だけ延ばす
	if (m_hitStopTimer <= 0.0f)
	{
		m_hitStopBasePosition = m_modelPosition;
		m_hitStopFrame = 0;
	}
	m_hitStopTimer = seconds;
	m_hitStopTime = seconds;
	m_hitStopAmplitude = amplitude;
}

void Character::UpdateHitStop()
{
	m_hitStopTimer = max(m_hitStopTimer - g_gameTime->GetFrameDeltaTime(), 0.0f);
	m_hitStopFrame++;

	// 画面の左右に震わせる（カメラの右の向き）。最初が大きく、止まり終わりに 0 へ近づける
	const float side = (m_hitStopFrame % 2 == 0) ? 1.0f : -1.0f;
	const float fade = (m_hitStopTime > 0.0f) ? m_hitStopTimer / m_hitStopTime : 0.0f;
	const Vector3 offset = g_camera3D->GetRight() * (side * m_hitStopAmplitude * fade);

	m_modelPosition = m_hitStopBasePosition + offset;
	m_modelRender.SetPosition(m_modelPosition);
	m_modelRender.Update();
}

void Character::Render(RenderContext& rc)
{
	m_modelRender.Draw(rc);
}

void Character::UpdateBody()
{
	const Vector3 position = m_charaCon.Execute(m_moveSpeed, g_gameTime->GetFrameDeltaTime());

	// 見た目だけ傾ける。足元ではなく m_modelTiltPivotHeight の高さを中心に回すので、その点が動かないように位置もずらす
	Quaternion modelRotation = m_rotation;
	modelRotation.Multiply(m_modelTilt);
	const Vector3 pivot(0.0f, m_modelTiltPivotHeight, 0.0f);
	Vector3 tiltedPivot = pivot;
	m_modelTilt.Apply(tiltedPivot);

	m_modelRotation = modelRotation;
	m_modelPosition = position + pivot - tiltedPivot;
	m_modelRender.SetRotation(m_modelRotation);
	m_modelRender.SetPosition(m_modelPosition);
	m_modelRender.Update();
}

void Character::ApplyGravity()
{
	m_gravity.Apply(m_moveSpeed);
}

void Character::ApproachHorizontal(const Vector3& target, const float rate)
{
	Vector3 diff = target - m_moveSpeed;
	diff.y = 0.0f;

	const float step = rate * g_gameTime->GetFrameDeltaTime();

	if (diff.LengthSq() <= step * step)
	{
		m_moveSpeed.x = target.x;
		m_moveSpeed.z = target.z;
		return;
	}

	diff.Normalize();
	m_moveSpeed.x += diff.x * step;
	m_moveSpeed.z += diff.z * step;
}

void Character::FaceDirection(const Vector3& dir)
{
	Vector3 faceDir = dir;
	faceDir.y = 0.0f;

	if (faceDir.LengthSq() < MIN_FACE_LENGTH_SQ)
	{
		return;
	}

	m_rotation.SetRotationYFromDirectionXZ(faceDir);
}

void Character::SetHorizontalSpeed(const Vector3& speed)
{
	m_moveSpeed.x = speed.x;
	m_moveSpeed.z = speed.z;
}

void Character::SetModelTiltTarget(const Quaternion& target, const float pivotHeight)
{
	m_modelTiltTarget = target;
	m_modelTiltPivotHeight = pivotHeight;
}

void Character::InitCharacter(const char* filePath, std::vector<AnimationClip>& clips, const float radius, const float height, const Vector3& position)
{
	m_modelRender.Init(filePath, clips);
	m_charaCon.Init(radius, height, position);
}

void Character::InitCharacter(const char* filePath, const float radius, const float height, const Vector3& position)
{
	m_modelRender.Init(filePath);
	m_charaCon.Init(radius, height, position);
}
