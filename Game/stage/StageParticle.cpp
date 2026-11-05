#include "stdafx.h"
#include "stage/StageParticle.h"

void StageParticle::Init()
{
	ModelInitData initData;
	initData.m_tkmFilePath = "Assets/modelData/debug/debug_wall.tkm";
	initData.m_fxFilePath = "Assets/shader/stageParticle.fx";
	initData.m_expandConstantBuffer = &m_constant;
	initData.m_expandConstantBufferSize = sizeof(m_constant);
	initData.m_colorBufferFormat[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
	initData.m_alphaBlendMode = AlphaBlendMode_Add;
	// 重なった粒が互いを隠さないように、深度は読むだけにする
	initData.m_isDepthWrite = false;
	m_model.Init(initData);
}

void StageParticle::Draw()
{
	m_constant.cameraPos = g_camera3D->GetPosition();
	m_constant.time += g_gameTime->GetFrameDeltaTime();
	g_renderingEngine->AddRenderObject(this);
}
