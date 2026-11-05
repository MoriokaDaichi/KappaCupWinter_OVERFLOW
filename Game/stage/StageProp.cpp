#include "stdafx.h"
#include "stage/StageProp.h"

void StageProp::Init(const char* tkmFilePath, const float tint)
{
	m_tint = tint;
	m_instances.reserve(StageParam::MAX_PROP);
	m_instanceBuffer.Init(sizeof(Instance), StageParam::MAX_PROP, nullptr);

	ModelInitData initData;
	initData.m_tkmFilePath = tkmFilePath;
	initData.m_fxFilePath = "Assets/shader/stageProp.fx";
	initData.m_expandShaderResoruceView[0] = &m_instanceBuffer;
	initData.m_expandConstantBuffer = &m_constant;
	initData.m_expandConstantBufferSize = sizeof(m_constant);
	initData.m_colorBufferFormat[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
	// 倍率やモデルの向きで面の表裏が変わっても消えないように、裏の面も描く
	initData.m_cullMode = D3D12_CULL_MODE_NONE;
	m_model.Init(initData);
}

void StageProp::Add(const StageParam::Prop& prop)
{
	Instance instance;
	instance.positionYaw.Set(prop.position[0], prop.position[1], prop.position[2], Math::DegToRad(prop.yaw));
	instance.scaleTint.Set(prop.scale[0], prop.scale[1], prop.scale[2], m_tint);
	m_instances.push_back(instance);
}

void StageProp::Draw()
{
	if (m_instances.empty())
	{
		return;
	}

	m_constant.cameraPos = g_camera3D->GetPosition();
	m_constant.time += g_gameTime->GetFrameDeltaTime();
	m_instanceBuffer.Update(m_instances.data());
	g_renderingEngine->AddRenderObject(this);
}
