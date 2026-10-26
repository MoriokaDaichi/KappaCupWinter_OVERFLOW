#include "stdafx.h"
#include "gimmick/Anchor.h"

bool Anchor::Start()
{
	m_modelRender.Init("Assets/modelData/debug/debug_anchor.tkm");
	m_modelRender.SetPosition(m_position);

	return true;
}

void Anchor::Update()
{
	const float scale = m_isLocked ? m_param.lockedScale : m_param.normalScale;
	m_modelRender.SetScale(scale, scale, scale);
	m_modelRender.Update();
}

void Anchor::Render(RenderContext& rc)
{
	m_modelRender.Draw(rc);
}
