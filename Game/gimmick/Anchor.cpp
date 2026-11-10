#include "stdafx.h"
#include "gimmick/Anchor.h"

bool Anchor::Start()
{
	// 飾りの多い場面でも目立つように、ライトに関係なく光らせる
	ModelInitData initData;
	initData.m_tkmFilePath = "Assets/modelData/stage/anchor.tkm";
	initData.m_fxFilePath = "Assets/shader/anchorGlow.fx";
	initData.m_colorBufferFormat[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
	m_modelRender.InitForwardRendering(initData);
	m_modelRender.SetPosition(m_position);

	return true;
}

void Anchor::Update()
{
	const float scale = m_isLocked ? m_param.lockedScale : m_param.normalScale;
	m_modelRender.SetPosition(m_position);
	m_modelRender.SetScale(scale, scale, scale);
	m_modelRender.Update();
}

void Anchor::Render(RenderContext& rc)
{
	m_modelRender.Draw(rc);
}
