#include "stdafx.h"
#include "hud/HudShape.h"

void HudShape::Init(const char* fxFilePath, const float width, const float height)
{
	SpriteInitData initData;
	// Sprite はテクスチャがないと作れないので、仮の照準の画像を渡しておく（シェーダーでは使わない）
	initData.m_ddsFilePath[0] = "Assets/sprite/debug/debug_crosshair.DDS";
	initData.m_fxFilePath = fxFilePath;
	initData.m_width = static_cast<UINT>(width);
	initData.m_height = static_cast<UINT>(height);
	initData.m_expandConstantBuffer = &m_shapeData;
	initData.m_expandConstantBufferSize = sizeof(m_shapeData);
	initData.m_alphaBlendMode = AlphaBlendMode_Trans;
	m_sprite.Init(initData);
}

void HudShape::Update(const Vector2& screenPos, const Vector4& color, const float value, const float flag)
{
	m_shapeData.color = color;
	m_shapeData.value = value;
	m_shapeData.flag = flag;
	m_shapeData.time += g_gameTime->GetFrameDeltaTime();
	m_sprite.Update(Vector3(screenPos.x, screenPos.y, 0.0f), Quaternion::Identity, Vector3::One);
}

void HudShape::Draw()
{
	g_renderingEngine->AddRenderObject(this);
}
