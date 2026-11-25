#include "stdafx.h"
#include "hud/JapaneseText.h"

std::wstring JapaneseText::ToWide(const std::string& utf8)
{
	if (utf8.empty())
	{
		return std::wstring();
	}
	const int length = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), static_cast<int>(utf8.size()), nullptr, 0);
	std::wstring wide(length, L'\0');
	MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), static_cast<int>(utf8.size()), &wide[0], length);
	return wide;
}

void JapaneseText::Init(const float height, const Align align)
{
	m_align = align;
	m_scale = height / FONT_LINE_HEIGHT;
	m_font.SetScale(m_scale);
	// 暗い所でも明るい所でも読めるように、細い影を付ける
	m_font.SetShadowParam(true, 2.0f, Vector4(0.0f, 0.0f, 0.05f, 0.9f));
}

void JapaneseText::SetText(const std::string& utf8)
{
	m_text = ToWide(utf8);
	SetVisibleCount(m_visibleCount);
}

void JapaneseText::SetVisibleCount(const int count)
{
	m_visibleCount = count;
	const size_t shown = (count < 0) ? m_text.size() : min(static_cast<size_t>(count), m_text.size());
	// FontRender の文字の箱は MAX_TEXT_SIZE まで
	const std::wstring visible = m_text.substr(0, min(shown, static_cast<size_t>(FontRender::MAX_TEXT_SIZE - 1)));
	m_font.SetText(visible.c_str());
}

void JapaneseText::SetPosition(const Vector2& position)
{
	// FontRender の位置は文字の左上。高さの真ん中と、そろえ方に合わせてずらす
	float x = position.x;
	if (m_align == Align::Center)
	{
		x -= FONT_CHAR_WIDTH * m_scale * static_cast<float>(m_text.size()) * 0.5f;
	}
	const float y = position.y + FONT_LINE_HEIGHT * m_scale * 0.5f;
	m_font.SetPosition(Vector3(x, y, 0.0f));
}
