#include "stdafx.h"
#include "hud/JapaneseText.h"

#include <fstream>
#include <iterator>
#include <unordered_map>

namespace
{
	constexpr const char* CHARSET_PATH = "Assets/sprite/hud/hud_font_jp.txt";	///< 画像に入っている文字の一覧（この順にマスへ並んでいる）
	constexpr int BLANK = -1;													///< 空白（描かない文字）の番号
	constexpr wchar_t FULL_WIDTH_SPACE = L'　';							///< 全角の空白
	constexpr wchar_t ASCII_FIRST = 0x21;										///< 全角に変える英数字・記号の範囲（! から ~ まで）
	constexpr wchar_t ASCII_LAST = 0x7E;
	constexpr wchar_t FULL_WIDTH_OFFSET = 0xFEE0;								///< 英数字・記号を全角にするときに足す数

	/** 文字から画像のマスの番号を引く表。最初に使うときに一度だけ読む */
	const std::unordered_map<wchar_t, int>& GetCodeTable()
	{
		static std::unordered_map<wchar_t, int> s_table;
		static bool s_isLoaded = false;
		if (!s_isLoaded)
		{
			s_isLoaded = true;
			std::ifstream file(CHARSET_PATH, std::ios::binary);
			const std::string utf8((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
			const std::wstring chars = JapaneseText::ToWide(utf8);
			for (int i = 0; i < static_cast<int>(chars.size()); i++)
			{
				s_table[chars[i]] = i;
			}
		}
		return s_table;
	}

	/** 英数字・記号・空白を全角にする（画像の 1 マスは正方形で、全角の形で入っている） */
	wchar_t ToFullWidth(const wchar_t ch)
	{
		if (ch == L' ')
		{
			return FULL_WIDTH_SPACE;
		}
		if (ch >= ASCII_FIRST && ch <= ASCII_LAST)
		{
			return static_cast<wchar_t>(ch + FULL_WIDTH_OFFSET);
		}
		return ch;
	}
}

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
	const HudText::Align hudAlign = (align == Align::Center) ? HudText::Align::Center : HudText::Align::Left;
	m_text.Init(HudText::MAX_LENGTH, height, hudAlign, HudText::Font::Japanese);
}

void JapaneseText::SetText(const std::string& utf8)
{
	m_wide = ToWide(utf8);
	for (wchar_t& ch : m_wide)
	{
		ch = ToFullWidth(ch);
	}
	UpdateCodes();
}

void JapaneseText::SetVisibleCount(const int count)
{
	m_visibleCount = count;
	UpdateCodes();
}

void JapaneseText::UpdateCodes()
{
	const std::unordered_map<wchar_t, int>& table = GetCodeTable();
	const int total = static_cast<int>(m_wide.size());
	const int shown = (m_visibleCount < 0) ? total : min(m_visibleCount, total);

	int codes[HudText::MAX_LENGTH] = {};
	const int count = min(shown, HudText::MAX_LENGTH);
	for (int i = 0; i < count; i++)
	{
		const auto found = table.find(m_wide[i]);
		// 画像にない文字（全角の空白も）は空白にする
		codes[i] = (found != table.end()) ? found->second : BLANK;
	}
	m_text.SetCodes(codes, count);
}
