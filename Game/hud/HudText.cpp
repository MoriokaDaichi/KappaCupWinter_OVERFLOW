#include "stdafx.h"
#include "hud/HudText.h"

namespace
{
	/** フォントの画像の文字の並び（D:\my\05_Assets\02_OVERFLOW\hud\gen_hud_font.py の CHARSET と同じにすること） */
	const char* CHARSET = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ:./[]-v";
	constexpr int BLANK = -1;						///< 空白（描かない文字）の番号

	/** 文字を、フォントの画像の中の番号にする。ない文字は空白 */
	int ToCode(const char ch)
	{
		if (ch == '\0' || ch == ' ')
		{
			return BLANK;
		}
		const char* found = strchr(CHARSET, ch);
		return (found != nullptr) ? static_cast<int>(found - CHARSET) : BLANK;
	}

	/** そろえ方を、シェーダーに渡す割合（0 で左、0.5 で真ん中、1 で右）にする */
	float ToAlignRate(const HudText::Align align)
	{
		switch (align)
		{
		case HudText::Align::Center:	return 0.5f;
		case HudText::Align::Right:		return 1.0f;
		default:						return 0.0f;
		}
	}
}

void HudText::Init(const int maxLength, const float height, const Align align)
{
	const int length = min(maxLength, MAX_LENGTH);
	m_align = align;
	m_width = height * CELL_ASPECT * length;

	m_textData.maxLength = static_cast<float>(length);
	m_textData.align = ToAlignRate(align);
	m_textData.glowStrength = GLOW_STRENGTH;
	SetColor(Vector4::White);

	SpriteInitData initData;
	initData.m_ddsFilePath[0] = "Assets/sprite/hud/hud_font.DDS";
	initData.m_fxFilePath = "Assets/shader/hudText.fx";
	initData.m_width = static_cast<UINT>(m_width);
	initData.m_height = static_cast<UINT>(height);
	initData.m_expandConstantBuffer = &m_textData;
	initData.m_expandConstantBufferSize = sizeof(m_textData);
	initData.m_alphaBlendMode = AlphaBlendMode_Trans;
	m_sprite.Init(initData);
}

void HudText::SetText(const char* text)
{
	const int length = static_cast<int>(m_textData.maxLength);
	int count = 0;
	for (; count < length && text[count] != '\0'; count++)
	{
		m_textData.chars[count] = ToCode(text[count]);
	}
	m_textData.charCount = static_cast<float>(count);
}

void HudText::SetColor(const Vector4& color)
{
	m_textData.color = color;
	m_textData.glowColor.Set(color.x, color.y, color.z, color.w * GLOW_ALPHA);
}

void HudText::SetPosition(const Vector2& position)
{
	// 板の位置は中心なので、そろえ方に合わせてずらす
	const float offset = (0.5f - ToAlignRate(m_align)) * m_width;
	m_sprite.Update(Vector3(position.x + offset, position.y, 0.0f), Quaternion::Identity, Vector3::One);
}

void HudText::Draw()
{
	g_renderingEngine->AddRenderObject(this);
}
