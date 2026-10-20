#include "stdafx.h"
#include "weapon/Bullet.h"

#include "collision/RayCast.h"

bool Bullet::Start()
{
	m_modelRender.Init("Assets/modelData/debug/debug_anchor.tkm");
	m_modelRender.SetPosition(m_position);
	m_modelRender.SetScale(m_param.bulletScale, m_param.bulletScale, m_param.bulletScale);

	m_lifeTimer = m_param.bulletLifeTime;

	return true;
}

void Bullet::Update()
{
	m_lifeTimer -= g_gameTime->GetFrameDeltaTime();
	//時間が切れたら消える
	if (m_lifeTimer <= 0.0f)
	{
		DeleteGO(this);
		return;
	}

	Vector3 nextPos = m_position + m_velocity * g_gameTime->GetFrameDeltaTime();
	//今の位置から次の位置まで線で調べる（速いので、点で調べると薄い壁をすり抜ける）
	RayHitInfo hit = nsCollision::RayCast(m_position, nextPos);
	if (hit.isHit)
	{
		DeleteGO(this);
		return;
	}

	m_position = nextPos;
	m_modelRender.SetPosition(m_position);
	m_modelRender.Update();
}

void Bullet::Render(RenderContext& rc)
{
	m_modelRender.Draw(rc);
}

void Bullet::Init(const Vector3& position, const Vector3& velocity)
{
	m_position = position;
	m_velocity = velocity;
}
