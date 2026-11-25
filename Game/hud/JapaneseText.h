#pragma once
#include "hud/HudText.h"
#include <string>

/**
 * 日本語の文字列 1 つ分。M PLUS 1 Code（SIL OFL）を画像にした Assets/sprite/hud/hud_font_jp.DDS で、HUD の英数字と同じようににじませて描く
 * 画像に入っている文字は Assets/sprite/hud/hud_font_jp.txt の順（tutorial.json とメニューの文字だけ）。入っていない文字は空白になる
 * 文字を足したら D:\my\05_Assets\02_OVERFLOW\font の gen_jp_charset.py → gen_jp_atlas.ps1 → gen_jp_glow.py で作り直す
 * 英数字と記号は全角にして描く（画像の 1 マスが正方形なので）
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

	/** height は文字の高さ（ピクセル） */
	void Init(const float height, const Align align);
	/** 出す文字列（UTF-8）を決める */
	void SetText(const std::string& utf8);
	/** 出す文字列のうち、先頭から count 文字だけを出す（1 文字ずつ打ち出す演出用。count が長さより大きければ全部） */
	void SetVisibleCount(const int count);
	/** 色。にじみも同じ色で、少し薄く出す */
	void SetColor(const Vector4& color) { m_text.SetColor(color); }
	/** 位置（画面の中央を 0,0 とし、上がプラス）。高さは文字の真ん中 */
	void SetPosition(const Vector2& position) { m_text.SetPosition(position); }
	/** 描く予約をする（毎フレーム呼ぶ。rc は使わないが、ほかの描くものと呼び方をそろえている） */
	void Draw(RenderContext& rc) { m_text.Draw(); }

	/** UTF-8 の文字列を wchar_t の文字列にする */
	static std::wstring ToWide(const std::string& utf8);

private:
	/** 今の文字列の先頭 m_visibleCount 文字を、フォントの画像の番号にして HudText に渡す */
	void UpdateCodes();

private:
	HudText m_text;					///< 描く係（日本語のフォントの画像を使う）
	std::wstring m_wide;			///< 全部の文字（全角に変えた後）
	int m_visibleCount = -1;		///< 出す文字の数（-1 なら全部）
};
