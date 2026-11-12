#include "stdafx.h"
#include "gimmick/GimmickView.h"

void GimmickView::Init(const char* tkmFilePath, const Kind kind, const int maxCount)
{
	m_maxCount = maxCount;
	m_constant.kind = static_cast<float>(kind);
	// StructuredBuffer は置ける数の分だけ読むので、中身ごと作っておく
	m_instances.resize(maxCount);
	m_instanceBuffer.Init(sizeof(Instance), maxCount, nullptr);

	ModelInitData initData;
	initData.m_tkmFilePath = tkmFilePath;
	initData.m_fxFilePath = "Assets/shader/gimmick.fx";
	initData.m_expandShaderResoruceView[0] = &m_instanceBuffer;
	initData.m_expandConstantBuffer = &m_constant;
	initData.m_expandConstantBufferSize = sizeof(m_constant);
	initData.m_colorBufferFormat[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
	m_model.Init(initData);
}

void GimmickView::Add(const Vector3& position, const Vector3& scale)
{
	if (m_count >= m_maxCount)
	{
		return;
	}

	Instance instance;
	instance.position.Set(position.x, position.y, position.z, 0.0f);
	instance.scale.Set(scale.x, scale.y, scale.z, 0.0f);
	m_instances[m_count] = instance;
	m_count++;
}

void GimmickView::Draw(const float brightness)
{
	if (m_count == 0)
	{
		return;
	}

	m_constant.cameraPos = g_camera3D->GetPosition();
	m_constant.time += g_gameTime->GetFrameDeltaTime();
	m_constant.brightness = brightness;
	m_instanceBuffer.Update(m_instances.data());
	g_renderingEngine->AddRenderObject(this);
}
