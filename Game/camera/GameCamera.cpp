#include "stdafx.h"
#include "camera/GameCamera.h"
#include "actor/BattleCharacter.h"
#include "input/InputManager.h"

bool GameCamera::Start()
{
	m_collisionSolver.Init(m_param.collisionRadius);
	g_camera3D->SetNear(m_param.nearClip);
	g_camera3D->SetFar(m_param.farClip);

	m_viewAngle = m_param.viewAngleNormal;
	if (m_player != nullptr)
	{
		m_lastLevel = m_player->GetSpeedSystem().GetLevel();
		m_pivotY = m_player->GetPosition().y + m_param.targetHeight;
		m_lastWarpCount = m_player->GetWarpCount();
	}

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
	const Vector2 stick = m_input->GetCamera();

	const float dt = g_gameTime->GetFrameDeltaTime();

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
	const float speed = horizontal.Length();

	float rate = speed / m_param.maxSpeedForFov;
	rate = min(rate, 1.0f);
	const float angle = Math::Lerp(rate, m_param.viewAngleNormal, m_param.viewAngleMax);

	m_viewAngle = Math::Lerp(min(m_param.viewAngleLerpRate * g_gameTime->GetFrameDeltaTime(), 1.0f), m_viewAngle, angle);

	// Lv が上がった瞬間に一気に広げ、はじめ速く後でゆっくり戻す（下がったときも覚え直す）
	const int level = m_player->GetSpeedSystem().GetLevel();
	if (level > m_lastLevel)
	{
		m_kickTimer = m_param.levelUpKickTime;
	}
	m_lastLevel = level;
	m_kickTimer = max(m_kickTimer - g_gameTime->GetFrameDeltaTime(), 0.0f);
	const float kickRate = (m_param.levelUpKickTime > 0.0f) ? m_kickTimer / m_param.levelUpKickTime : 0.0f;
	const float kick = m_param.levelUpViewAngleKick * kickRate * kickRate;

	const float viewAngle = m_viewAngle + kick;

	g_camera3D->SetViewAngle(Math::DegToRad(viewAngle));
}

void GameCamera::UpdatePosition()
{
	const float distance = m_param.distance;
	const float shoulderOffset = m_param.shoulderOffset;

	Vector3 pivot = m_player->GetPosition();
	pivot.y += m_param.targetHeight;

	if (m_player->GetWarpCount() != m_lastWarpCount)
	{
		m_pivotY = pivot.y;
		m_lastWarpCount = m_player->GetWarpCount();
	}

	const float followRate = (m_player->GetCurrentState() == EnCharacterState::Swing) ? m_param.swingPivotFollowRate : m_param.pivotFollowRate;
	m_pivotY = Math::Lerp(min(followRate * g_gameTime->GetFrameDeltaTime(), 1.0f), m_pivotY, pivot.y);

	const float gap = m_pivotY - pivot.y;
	if (gap > m_param.pivotMaxLag)
	{
		m_pivotY = pivot.y + m_param.pivotMaxLag;
	}
	else if (gap < -m_param.pivotMaxLag)
	{
		m_pivotY = pivot.y - m_param.pivotMaxLag;
	}

	pivot.y = m_pivotY;

	Vector3 toCamera(0.0f, 0.0f, -1.0f);

	Quaternion qPitch;
	qPitch.SetRotationDegX(-m_pitch);
	Quaternion qYaw;
	qYaw.SetRotationDegY(m_yaw);

	qPitch.Apply(toCamera);
	qYaw.Apply(toCamera);

	Vector3 right(1.0f, 0.0f, 0.0f);
	qYaw.Apply(right);

	const Vector3 shoulder = pivot + right * shoulderOffset;
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

	const Vector3 position = shoulderResult + toCamera * distance;

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
