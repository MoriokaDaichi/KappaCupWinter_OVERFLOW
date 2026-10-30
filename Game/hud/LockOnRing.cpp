#include "stdafx.h"
#include "hud/LockOnRing.h"

void LockOnRing::Init(const float size)
{
	m_ringData.thickness = RING_THICKNESS;

	SpriteInitData initData;
	// Sprite はテクスチャがないと作れないので、仮の照準の画像を渡しておく（シェーダーでは使わない）
	initData.m_ddsFilePath[0] = "Assets/sprite/debug/debug_crosshair.DDS";
	initData.m_fxFilePath = "Assets/shader/lockOnRing.fx";
	initData.m_width = static_cast<UINT>(size);
	initData.m_height = static_cast<UINT>(size);
	initData.m_expandConstantBuffer = &m_ringData;
	initData.m_expandConstantBufferSize = sizeof(m_ringData);
	initData.m_alphaBlendMode = AlphaBlendMode_Trans;
	m_sprite.Init(initData);
}

void LockOnRing::Update(const Vector2& screenPos, const float progress, const Vector4& color)
{
	m_ringData.color = color;
	m_ringData.progress = progress;
	m_sprite.Update(Vector3(screenPos.x, screenPos.y, 0.0f), Quaternion::Identity, Vector3::One);
}

void LockOnRing::Draw(RenderContext& rc)
{
	g_renderingEngine->AddRenderObject(this);
}
