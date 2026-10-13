#include "stdafx.h"
#include "actor/Character.h"

void Character::Update()
{
	UpdateAction();
	UpdateBody();
}

void Character::Render(RenderContext& rc)
{
	m_modelRender.Draw(rc);
}

void Character::UpdateBody()
{
	Vector3 position = m_charaCon.Execute(m_moveSpeed, g_gameTime->GetFrameDeltaTime());

	m_modelRender.SetRotation(m_rotation);
	m_modelRender.SetPosition(position);
	m_modelRender.Update();
}

void Character::ApplyGravity()
{
	m_gravity.Apply(m_moveSpeed);
}

void Character::InitCharacter(const char* filePath, float radius, float height, const Vector3& position)
{
	m_modelRender.Init(filePath);
	m_charaCon.Init(radius, height, position);
}
