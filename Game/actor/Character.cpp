#include "stdafx.h"
#include "actor/Character.h"

void Character::Update()
{
	UpdateAction();
	UpdateBody();
	OnBodyUpdated();
}

void Character::Render(RenderContext& rc)
{
	m_modelRender.Draw(rc);
}

void Character::UpdateBody()
{
	Vector3 position = m_charaCon.Execute(m_moveSpeed, g_gameTime->GetFrameDeltaTime());

	// 見た目だけ傾ける。足元ではなく m_modelTiltPivotHeight の高さを中心に回すので、その点が動かないように位置もずらす
	Quaternion modelRotation = m_rotation;
	modelRotation.Multiply(m_modelTilt);
	Vector3 pivot(0.0f, m_modelTiltPivotHeight, 0.0f);
	Vector3 tiltedPivot = pivot;
	m_modelTilt.Apply(tiltedPivot);

	m_modelRender.SetRotation(modelRotation);
	m_modelRender.SetPosition(position + pivot - tiltedPivot);
	m_modelRender.Update();
}

void Character::ApplyGravity()
{
	m_gravity.Apply(m_moveSpeed);
}

void Character::ApproachHorizontal(const Vector3& target, float rate)
{
	Vector3 diff = target - m_moveSpeed;
	diff.y = 0.0f;

	float step = rate * g_gameTime->GetFrameDeltaTime();

	if (diff.Length() <= step)
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

void Character::InitCharacter(const char* filePath, AnimationClip* clips, int clipCount, float radius, float height, const Vector3& position)
{
	m_modelRender.Init(filePath, clips, clipCount);
	m_charaCon.Init(radius, height, position);
}
