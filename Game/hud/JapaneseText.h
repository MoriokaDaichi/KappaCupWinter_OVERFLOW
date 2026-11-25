#pragma once
#include <string>

/**
 * 日本語の文字列 1 つ分。エンジンの FontRender（Assets/font/myfile.spritefont。日本語が入っている）で描く
 * HudText（自作のデジタルな字形）は英数字と記号しか出せないので、日本語の案内とメニューはこちらを使う
 * 文字列は UTF-8 で受け取り、FontRender が使う wchar_t に変えて持つ
 */
class JapaneseText
{
public:
	/** そろえ方。位置を、文字列のどこに合わせるか */
	enum class Align : int
	{
		Left,		///< 左端
		Center,		///< 真ん中
	};

	/** height は文字の高さ（ピクセル）。フォントの 1 行の高さに対する倍率にして FontRender に渡す */
	void Init(const float height, const Align align);
	/** 出す文字列（UTF-8）を決める */
	void SetText(const std::string& utf8);
	/** 出す文字列のうち、先頭から count 文字だけを出す（1 文字ずつ打ち出す演出用。count が長さより大きければ全部） */
	void SetVisibleCount(const int count);
	/** 文字の数（UTF-16 の数。日本語は 1 文字 1 つ） */
	int GetLength() const { return static_cast<int>(m_text.size()); }
	/** 色 */
	void SetColor(const Vector4& color) { m_font.SetColor(color); }
	/** 位置（画面の中央を 0,0 とし、上がプラス）。高さは文字の真ん中 */
	void SetPosition(const Vector2& position);
	/** 描く予約をする（毎フレーム呼ぶ） */
	void Draw(RenderContext& rc) { m_font.Draw(rc); }

	/** UTF-8 の文字列を wchar_t の文字列にする（tutorial.json の文字を読むときにも使う） */
	static std::wstring ToWide(const std::string& utf8);

private:
	static constexpr float FONT_LINE_HEIGHT = 42.7f;	///< フォントの 1 行の高さ（倍率 1 のとき。myfile.spritefont の値）
	static constexpr float FONT_CHAR_WIDTH = 40.0f;		///< 日本語 1 文字の幅（倍率 1 のとき）。真ん中にそろえるときの目安

	FontRender m_font;				///< 描く係
	std::wstring m_text;			///< 全部の文字
	Align m_align = Align::Left;	///< そろえ方
	float m_scale = 1.0f;			///< フォントの倍率
	int m_visibleCount = -1;		///< 出す文字の数（-1 なら全部）
};
