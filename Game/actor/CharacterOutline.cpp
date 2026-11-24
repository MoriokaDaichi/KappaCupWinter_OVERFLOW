#include "stdafx.h"
#include "actor/CharacterOutline.h"

void CharacterOutline::Init(const char* modelPath, const CharacterParam::Outline& param)
{
	m_constant.width = param.width;
	m_constant.color = Vector3(param.color[0], param.color[1], param.color[2]);

	ModelInitData initData;
	initData.m_tkmFilePath = modelPath;
	initData.m_fxFilePath = "Assets/shader/outline.fx";
	initData.m_expandConstantBuffer = &m_constant;
	initData.m_expandConstantBufferSize = sizeof(m_constant);
	// 表の面を消して裏の面だけ描く。ふくらませた殻の手前側は消えるので体を隠さず、ふちだけが体の外に残る
	initData.m_cullMode = D3D12_CULL_MODE_FRONT;
	// 光らせる（1 より大きい色を Bloom に渡す）ので、明るさを切り捨てない形式で描く（Anchor と同じ）
	initData.m_colorBufferFormat[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
	m_modelRender.InitForwardRendering(initData);
	// 殻の影は体の影と重なるだけなので落とさない
	m_modelRender.SetShadowCasterFlag(false);
}

void CharacterOutline::Update(const Vector3& position, const Quaternion& rotation, const float scale)
{
	m_constant.cameraPos = g_camera3D->GetPosition();

	m_modelRender.SetPosition(position);
	m_modelRender.SetRotation(rotation);
	m_modelRender.SetScale(scale, scale, scale);
	m_modelRender.Update();
}

void CharacterOutline::Draw(RenderContext& rc)
{
	m_modelRender.Draw(rc);
}
