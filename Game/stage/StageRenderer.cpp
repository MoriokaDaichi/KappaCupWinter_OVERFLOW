#include "stdafx.h"
#include "stage/StageRenderer.h"

void StageRenderer::Init()
{
	m_instances[0].halfSize.Set(BACKGROUND_SIZE, BACKGROUND_SIZE, BACKGROUND_SIZE, 0.0f);
	m_instanceBuffer.Init(sizeof(Instance), MAX_INSTANCE, nullptr);
	m_wallMask.InitFromDDSFile(L"Assets/modelData/stage/stage_wall_mask.DDS");

	ModelInitData initData;
	initData.m_tkmFilePath = "Assets/modelData/debug/debug_wall.tkm";
	initData.m_fxFilePath = "Assets/shader/stageBlock.fx";
	initData.m_expandShaderResoruceView[0] = &m_instanceBuffer;
	initData.m_expandShaderResoruceView[1] = &m_wallMask;
	initData.m_expandConstantBuffer = &m_constant;
	initData.m_expandConstantBufferSize = sizeof(m_constant);
	initData.m_colorBufferFormat[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
	// 背景はカメラが中に入るので、裏の面も描く
	initData.m_cullMode = D3D12_CULL_MODE_NONE;
	m_model.Init(initData);

	// エンジンの自動露出は、暗い場面を明るく持ち上げて紺を白っぽくする。露出を下げ、そのぶんキャラに当たる光を強くする
	// ステージは +Z へ進むので、カメラから見える背中と、こちらを向く敵に当たるように、ライトも +Z へ向ける
	g_renderingEngine->SetSceneMiddleGray(g_renderingEngine->GetSceneMiddleGray() * EXPOSURE_SCALE);
	Vector3 lightDirection(LIGHT_DIRECTION_X, LIGHT_DIRECTION_Y, LIGHT_DIRECTION_Z);
	lightDirection.Normalize();
	g_renderingEngine->SetDirectionLight(0, lightDirection, Vector3(LIGHT_BRIGHTNESS, LIGHT_BRIGHTNESS, LIGHT_BRIGHTNESS));
}

void StageRenderer::AddBlock(const Vector3& position, const Vector3& size, const Kind kind)
{
	Instance& instance = m_instances[m_instanceCount];
	instance.center.Set(position.x, position.y, position.z, 0.0f);
	instance.halfSize.Set(size.x * 0.5f, size.y * 0.5f, size.z * 0.5f, static_cast<float>(kind));
	m_instanceCount++;
}

void StageRenderer::Draw()
{
	const Vector3& cameraPos = g_camera3D->GetPosition();
	m_instances[0].center.Set(cameraPos.x, cameraPos.y, cameraPos.z, 0.0f);

	m_constant.time += g_gameTime->GetFrameDeltaTime();
	m_instanceBuffer.Update(m_instances.data());
	g_renderingEngine->AddRenderObject(this);
}
