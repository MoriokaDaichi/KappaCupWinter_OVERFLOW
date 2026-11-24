/*!
 * @brief	タイトルとリザルトの背景。紺の空に、手前へ流れてくる光る格子の床（電脳空間）と、地平線の光、走査線
 * shapeColor で格子の色、value で格子の流れる速さを変える
 */

static const float HORIZON = 0.58f;				// 地平線の高さ（上が 0、下が 1）
static const float GRID_DEPTH_SCALE = 0.35f;	// 奥行きの格子の間隔
static const float GRID_SIDE_SCALE = 6.0f;		// 横の格子の間隔
static const float LINE_WIDTH = 0.04f;			// 格子の線の太さ（格子 1 つに対する割合）
static const float FAR_FADE = 6.0f;				// この奥行きより奥の格子は消えていく
static const float GLOW_WIDTH = 0.012f;			// 地平線の光の太さ
static const float SCANLINE_COUNT = 360.0f;		// 走査線の本数
static const float SCANLINE_DEPTH = 0.12f;		// 走査線で暗くなる割合
static const float3 SKY_TOP = float3(0.01f, 0.01f, 0.05f);		// 空の上の色
static const float3 SKY_BOTTOM = float3(0.05f, 0.03f, 0.15f);	// 空の地平線の近くの色

cbuffer cb : register(b0){
	float4x4 mvp;		// モデルビュープロジェクション行列
	float4 mulColor;	// 乗算カラー（使わない）
};

// HudShape の ShapeData と同じ並び
cbuffer ShapeCb : register(b1){
	float4 shapeColor;	// 格子と地平線の色
	float value;		// 格子の流れる速さ（1 秒に格子いくつ分）
	float time;			// 経過時間（秒）
	float flag;			// 使わない
};

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

/** 0〜1 をくり返す座標 f が、格子の線の上なら 1 に近い値を返す */
float GridLine(float f)
{
	const float d = min(f, 1.0f - f);
	return 1.0f - smoothstep(0.0f, LINE_WIDTH, d);
}

float4 PSMain(PSInput In) : SV_Target0
{
	const float2 uv = In.uv;
	float3 color = lerp(SKY_TOP, SKY_BOTTOM, saturate(uv.y / HORIZON));

	// 床：地平線より下。下へ行くほど手前（奥行き depth が小さい）
	const float below = uv.y - HORIZON;
	if (below > 0.0f)
	{
		const float depth = 1.0f / (below * 8.0f);
		const float side = (uv.x - 0.5f) * depth;
		const float lineZ = GridLine(frac(depth * GRID_DEPTH_SCALE + time * value * GRID_DEPTH_SCALE));
		const float lineX = GridLine(frac(side * GRID_SIDE_SCALE * GRID_DEPTH_SCALE));
		const float fade = saturate(1.0f - depth / FAR_FADE);
		color = float3(0.02f, 0.01f, 0.06f) + shapeColor.rgb * max(lineZ, lineX) * fade;
	}

	// 地平線の光
	const float glow = exp(-abs(below) / GLOW_WIDTH);
	color += shapeColor.rgb * glow * 0.8f;

	// 走査線
	color *= 1.0f - SCANLINE_DEPTH * (0.5f + 0.5f * sin(uv.y * SCANLINE_COUNT * 3.14159f));

	// 四隅を暗くする
	const float2 fromCenter = uv - 0.5f;
	color *= 1.0f - dot(fromCenter, fromCenter) * 1.2f;

	return float4(color, 1.0f);
}
