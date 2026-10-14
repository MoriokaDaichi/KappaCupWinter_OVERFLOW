#include "stdafx.h"
#include "camera/GameCamera.h"
#include "actor/Player.h"
#include "InputManager.h"

bool GameCamera::Start()
{
	m_collisionSolver.Init(COLLISION_RADIUS);
	g_camera3D->SetNear(CAMERA_NEAR);
	g_camera3D->SetFar(CAMERA_FAR);

	m_viewAngle = VIEW_ANGLE_NORMAL;

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
	m_yaw += stick.x * ROTATE_SPEED_X * dt;
	m_pitch += stick.y * ROTATE_SPEED_Y * dt;

	if (m_pitch < PITCH_MIN)
	{
		m_pitch = PITCH_MIN;
	}
	
	if (m_pitch > PITCH_MAX)
	{
		m_pitch = PITCH_MAX;
	}
}

void GameCamera::UpdateViewAngle()
{
	Vector3 horizontal = m_player->GetMoveSpeed();
	horizontal.y = 0.0f;
	float speed = horizontal.Length();

	float rate = speed / MAX_SPEED_FOR_FOV;
	rate = min(rate, 1.0f);
	float angle = Math::Lerp(rate, VIEW_ANGLE_NORMAL, VIEW_ANGLE_MAX);

	m_viewAngle = Math::Lerp(min(VIEW_ANGLE_LERP_RATE * g_gameTime->GetFrameDeltaTime(), 1.0f), m_viewAngle, angle);

	g_camera3D->SetViewAngle(Math::DegToRad(m_viewAngle));
}

void GameCamera::UpdatePosition()
{
	Vector3 target = m_player->GetPosition();
	target.y += TARGET_HEIGHT;

	Vector3 toCamera(0.0f, 0.0f, -1.0f);

	Quaternion qPitch;
	qPitch.SetRotationDegX(-m_pitch);
	Quaternion qYaw;
	qYaw.SetRotationDegY(m_yaw);

	qPitch.Apply(toCamera);
	qYaw.Apply(toCamera);

	Vector3 position = target + toCamera * CAMERA_DISTANCE;

	Vector3 result;
	m_collisionSolver.Execute(result, position, target);

	g_camera3D->SetTarget(target);
	g_camera3D->SetPosition(result);
}
