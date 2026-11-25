/*!
 * @brief	HUD の日本語の文字（hudText.fx の日本語版）。M PLUS 1 Code を画像にした hud_font_jp.DDS から、文字を 1 つずつ並べて描く
 * 画像の R が文字、G が文字のまわりのにじみ。並びは Assets/sprite/hud/hud_font_jp.txt の順（JapaneseText が番号に変える）
 */

static const float ATLAS_COLS = 16.0f;		// 画像の横のマスの数
static const float ATLAS_ROWS = 8.0f;		// 画像の縦のマスの数（16 x 8。gen_jp_atlas.ps1 と同じ）
static const float MIP_SHARPEN = 0.5f;		// 縮小の画像を選ぶときの変化量に掛ける数（1 より小さいほど細かい画像を使い、くっきりする）
static const float EDGE_LOW = 0.3f;		// 文字のふちをくっきりさせる範囲（この濃さ以下は透明、EDGE_HIGH 以上は不透明）
static const float EDGE_HIGH = 0.7f;
static const float GLOW_SCALE = 0.45f;		// にじみの強さに掛ける数（日本語は線が込み入っているので、英数字より弱くする）
static const float3 GLOW_TINT = float3(0.0f, 0.9f, 1.0f);	// にじみの色（文字と同じ白だとぼやけて見えるので、シアンの光にする）
static const float GLOW_TINT_RATE = 0.8f;	// にじみの色を GLOW_TINT に寄せる割合

cbuffer cb : register(b0){
	float4x4 mvp;		// モデルビュープロジェクション行列
	float4 mulColor;	// 乗算カラー（使わない）
};

// HudText の TextData と同じ並び
cbuffer TextCb : register(b1){
	float4 textColor;		// 文字の色
	float4 glowColor;		// にじみの色
	int4 chars[8];			// 文字の番号（32 文字まで。-1 は空白。HudText::MAX_LENGTH と同じ）
	float charCount;		// 文字の数
	float maxLength;		// 板に並べられる文字の数
	float align;			// そろえ方（0 で左、0.5 で真ん中、1 で右）
	float glowStrength;		// にじみの強さ
};

Texture2D<float4> fontTexture : register(t0);
sampler Sampler : register(s0);

struct VSInput{
	float4 pos : POSITION;
	float2 uv  : TEXCOORD0;
};

struct PSInput{
	float4 pos : SV_POSITION;
	float2 uv  : TEXCOORD0;
};

PSInput VSMain(VSInput In)
{
	PSInput psIn;
	psIn.pos = mul(mvp, In.pos);
	psIn.uv = In.uv;
	return psIn;
}

float4 PSMain(PSInput In) : SV_Target0
{
	// 板の左から何文字目か（そろえ方の分だけずらす）
	const float x = In.uv.x * maxLength - align * (maxLength - charCount);
	const int index = (int)floor(x);
	if (index < 0 || index >= (int)charCount)
	{
		discard;
	}

	const int code = chars[index / 4][index % 4];
	if (code < 0)
	{
		discard;
	}

	const float2 cell = float2(code % (int)ATLAS_COLS, code / (int)ATLAS_COLS);
	const float2 atlasSize = float2(ATLAS_COLS, ATLAS_ROWS);
	const float2 uv = (cell + float2(frac(x), In.uv.y)) / atlasSize;

	// 文字の境目で縮小のぼけ具合が飛ばないように、つながった座標の変化量で読む
	const float2 continuous = float2(x, In.uv.y) / atlasSize;
	// 少し細かい縮小の画像を読む（そのままだと縮めたときに甘くなる）
	const float4 font = fontTexture.SampleGrad(Sampler, uv, ddx(continuous) * MIP_SHARPEN, ddy(continuous) * MIP_SHARPEN);

	const float body = smoothstep(EDGE_LOW, EDGE_HIGH, font.r);
	const float glow = font.g * glowStrength * GLOW_SCALE;
	const float alpha = max(body, glow);
	if (alpha <= 0.0f)
	{
		discard;
	}
	const float3 glowRgb = lerp(glowColor.rgb, GLOW_TINT, GLOW_TINT_RATE);
	const float3 color = lerp(glowRgb, textColor.rgb, body);
	return float4(color, alpha * textColor.a);
}
