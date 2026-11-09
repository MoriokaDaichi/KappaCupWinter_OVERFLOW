/*!
 * @brief	HUD の文字。自作フォントの画像（Assets/sprite/hud/hud_font.DDS）から、文字を 1 つずつ並べて描く
 * 画像の R が文字、G が文字のまわりのにじみ。並びは HudText.cpp の CHARSET と同じ
 */

static const float ATLAS_COLS = 16.0f;		// 画像の横のマスの数
static const float ATLAS_ROWS = 4.0f;		// 画像の縦のマスの数

cbuffer cb : register(b0){
	float4x4 mvp;		// モデルビュープロジェクション行列
	float4 mulColor;	// 乗算カラー（使わない）
};

// HudText の TextData と同じ並び
cbuffer TextCb : register(b1){
	float4 textColor;		// 文字の色
	float4 glowColor;		// にじみの色
	int4 chars[4];			// 文字の番号（16 文字まで。-1 は空白）
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
	const float4 font = fontTexture.SampleGrad(Sampler, uv, ddx(continuous), ddy(continuous));

	const float glow = font.g * glowStrength;
	const float alpha = max(font.r, glow);
	if (alpha <= 0.0f)
	{
		discard;
	}
	const float3 color = lerp(glowColor.rgb, textColor.rgb, saturate(font.r));
	return float4(color, alpha * textColor.a);
}
