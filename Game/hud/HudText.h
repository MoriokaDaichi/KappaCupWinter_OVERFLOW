#pragma once

/**
 * HUD の文字列 1 つ分。自作フォントの画像（Assets/sprite/hud/hud_font.DDS）から、文字を並べて 1 枚の板に描く（Assets/shader/hudText.fx）
 * 使える文字は数字・英大文字・「: . / [ ] -」・小文字の v・空白だけ。それ以外は空白になる
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

	/**
	 * @param maxLength	いちばん長いときの文字数（MAX_LENGTH まで）
	 * @param height	文字の高さ（ピクセル）
	 * @param align		そろえ方
	 */
	void Init(const int maxLength, const float height, const Align align);
	/** 出す文字列を決める。maxLength より長い分は出ない */
	void SetText(const char* text);
	/** 文字の色。にじみも同じ色で、少し薄く出す */
	void SetColor(const Vector4& color);
	/** 位置（画面の中央を 0,0 とする）。そろえ方の端または真ん中、高さは文字の真ん中 */
	void SetPosition(const Vector2& position);
	/** 描く予約をする（毎フレーム呼ぶ。呼ばなかったフレームは出ない） */
	void Draw();

	static constexpr int MAX_LENGTH = 16;		///< 1 つの文字列に出せる文字の数

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

	static constexpr float CELL_ASPECT = 64.0f / 96.0f;	///< フォントの画像の 1 マスの、幅÷高さ
	static constexpr float GLOW_STRENGTH = 0.8f;		///< にじみの強さ
	static constexpr float GLOW_ALPHA = 0.7f;			///< にじみの色の濃さ（文字の色に掛ける）

	Sprite m_sprite;					///< 文字を描く板
	TextData m_textData;				///< m_sprite がここを毎回シェーダーへ送る（Init の後は動かさない）
	Align m_align = Align::Left;		///< そろえ方
	float m_width = 0.0f;				///< 板の幅（ピクセル）
};
