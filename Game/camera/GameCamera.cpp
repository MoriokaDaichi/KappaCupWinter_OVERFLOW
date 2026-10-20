#include "stdafx.h"
#include "camera/GameCamera.h"
#include "actor/Player.h"
#include "input/InputManager.h"

bool GameCamera::Start()
{
	m_collisionSolver.Init(m_param.collisionRadius);
	g_camera3D->SetNear(m_param.nearClip);
	g_camera3D->SetFar(m_param.farClip);

	m_viewAngle = m_param.viewAngleNormal;

	return true;
}

void GameCamera::Update()
{
	if (m_player == nullptr || m_input == nullptr)
	{
		return;
	}

	Rotate();
	UpdateViewAngle();
	UpdatePosition();

	g_camera3D->Update();
}

void GameCamera::Rotate()
{
	Vector2 stick = m_input->GetCamera();

	float dt = g_gameTime->GetFrameDeltaTime();
	m_yaw += stick.x * m_param.rotateSpeedX * dt;
	m_pitch += stick.y * m_param.rotateSpeedY * dt;

	if (m_pitch < m_param.pitchMin)
	{
		m_pitch = m_param.pitchMin;
	}
	
	if (m_pitch > m_param.pitchMax)
	{
		m_pitch = m_param.pitchMax;
	}
}

void GameCamera::UpdateViewAngle()
{
	Vector3 horizontal = m_player->GetMoveSpeed();
	horizontal.y = 0.0f;
	float speed = horizontal.Length();

	float rate = speed / m_param.maxSpeedForFov;
	rate = min(rate, 1.0f);
	float angle = Math::Lerp(rate, m_param.viewAngleNormal, m_param.viewAngleMax);

	m_viewAngle = Math::Lerp(min(m_param.viewAngleLerpRate * g_gameTime->GetFrameDeltaTime(), 1.0f), m_viewAngle, angle);

	g_camera3D->SetViewAngle(Math::DegToRad(m_viewAngle));
}

void GameCamera::UpdatePosition()
{
	Vector3 pivot = m_player->GetPosition();
	pivot.y += m_param.targetHeight;

	Vector3 toCamera(0.0f, 0.0f, -1.0f);

	Quaternion qPitch;
	qPitch.SetRotationDegX(-m_pitch);
	Quaternion qYaw;
	qYaw.SetRotationDegY(m_yaw);

	qPitch.Apply(toCamera);
	qYaw.Apply(toCamera);

	Vector3 right(1.0f, 0.0f, 0.0f);
	qYaw.Apply(right);

	Vector3 shoulder = pivot + right * m_param.shoulderOffset;
	Vector3 shoulderResult;

	// 壁に当たったら、肩の点を体の中心の方へ少し戻す
	// （壁に接したままだと、2回目の壁よけがすぐ当たって、カメラと注視点が重なる）
	if (m_collisionSolver.Execute(shoulderResult, shoulder, pivot))
	{
		Vector3 toPivot = pivot - shoulderResult;
		if (toPivot.LengthSq() > SHOULDER_WALL_MARGIN * SHOULDER_WALL_MARGIN)
		{
			toPivot.Normalize();
			shoulderResult += toPivot * SHOULDER_WALL_MARGIN;
		}
		else
		{
			shoulderResult = pivot;
		}
	}

	Vector3 position = shoulderResult + toCamera * m_param.distance;

	Vector3 result;
	m_collisionSolver.Execute(result, position, shoulderResult);

	// 念のため、カメラと注視点が重なったら少し後ろへずらす（重なると DirectX が向きを出せずに止まる）
	if ((result - shoulderResult).LengthSq() < CAMERA_MIN_DISTANCE * CAMERA_MIN_DISTANCE)
	{
		result = shoulderResult + toCamera * CAMERA_MIN_DISTANCE;
	}

	g_camera3D->SetTarget(shoulderResult);
	g_camera3D->SetPosition(result);
}
