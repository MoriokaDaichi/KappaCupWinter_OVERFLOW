#pragma once

/**
 * HUD の文字列 1 つ分。自作フォントの画像（Assets/sprite/hud/hud_font.DDS）から、文字を並べて 1 枚の板に描く（Assets/shader/hudText.fx）
 * 使える文字は数字・英大文字・「: . / [ ] - > < _ ! # = , ( ) % * + ?」・小文字の v・空白だけ。それ以外は空白になる
 * 日本語のフォント（Font::Japanese。hud_font_jp.DDS、hudTextJp.fx）でも描ける。そのときは JapaneseText が文字の番号を SetCodes で渡す
 */
class HudText : public IRenderer
{
public:
	/** そろえ方。位置を、文字列のどこに合わせるか */
	enum class Align : int
	{
		Left,		///< 左端
		Center,		///< 真ん中
		Right,		///< 右端
	};

	/** どのフォントの画像で描くか */
	enum class Font : int
	{
		Digital,	///< 自作のデジタルな英数字（hud_font.DDS。1 マスは縦長）
		Japanese,	///< 日本語（M PLUS 1 Code を画像にした hud_font_jp.DDS。1 マスは正方形）
	};

	/**
	 * @param maxLength	いちばん長いときの文字数（MAX_LENGTH まで）
	 * @param height	文字の高さ（ピクセル）
	 * @param align		そろえ方
	 * @param font		どのフォントの画像で描くか
	 */
	void Init(const int maxLength, const float height, const Align align, const Font font = Font::Digital);
	/** 出す文字列を決める。maxLength より長い分は出ない */
	void SetText(const char* text);
	/** 文字の番号（フォントの画像のマスの番号。-1 は空白）を、そのまま渡す（日本語の JapaneseText が使う） */
	void SetCodes(const int* codes, const int count);
	/** 文字の色。にじみも同じ色で、少し薄く出す */
	void SetColor(const Vector4& color);
	/**
	 * 位置（画面の中央を 0,0 とする）。そろえ方の端または真ん中、高さは文字の真ん中
	 * scale で大きさを変える（演出用。そろえ方の端は動かさない）
	 */
	void SetPosition(const Vector2& position, const float scale = 1.0f);
	/** 描く予約をする（毎フレーム呼ぶ。呼ばなかったフレームは出ない） */
	void Draw();

	static constexpr int MAX_LENGTH = 32;		///< 1 つの文字列に出せる文字の数（hudText.fx の chars の数 × 4 と同じ）

private:
	/** 2D の描画のときに呼ばれる */
	void OnRender2D(RenderContext& rc) override { m_sprite.Draw(rc); }

private:
	/** シェーダーに渡す値（hudText.fx の TextCb と同じ並び。16 バイトずつにそろえる） */
	struct TextData
	{
		Vector4 color;						///< 文字の色
		Vector4 glowColor;					///< にじみの色
		int chars[MAX_LENGTH] = {};			///< 文字の番号（-1 は空白）
		float charCount = 0.0f;				///< 文字の数
		float maxLength = 0.0f;				///< 板に並べられる文字の数
		float align = 0.0f;					///< そろえ方（0 で左、0.5 で真ん中、1 で右）
		float glowStrength = 0.0f;			///< にじみの強さ
	};
	// cbuffer は 16 バイトずつ読むので、大きさを 16 の倍数にする（足りなければ padding を足す）
	static_assert(sizeof(TextData) % 16 == 0, "cbuffer size must be a multiple of 16 (add padding)");

	static constexpr float CELL_ASPECT = 64.0f / 96.0f;	///< デジタルのフォントの画像の 1 マスの、幅÷高さ（日本語は 1）
	static constexpr float GLOW_STRENGTH = 0.8f;		///< にじみの強さ
	static constexpr float GLOW_ALPHA = 0.7f;			///< にじみの色の濃さ（文字の色に掛ける）

	Sprite m_sprite;					///< 文字を描く板
	TextData m_textData;				///< m_sprite がここを毎回シェーダーへ送る（Init の後は動かさない）
	Align m_align = Align::Left;		///< そろえ方
	float m_width = 0.0f;				///< 板の幅（ピクセル）
};
