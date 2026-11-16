/*!
 * @brief	案内（チュートリアル）の帯。斜めに切った紺の帯に、上下のシアンの線と、左のマゼンタの印を付ける
 * value で左から帯を出していく（0 で見えない、1 で全部）。薄くするときは色の a を下げる
 */

static const float SLANT = 0.06f;			// 斜めの切り口の傾き（板の幅に対する、上下の端のずれ）
static const float EDGE_LINE = 0.06f;		// 上下の線の太さ（板の高さに対する割合）
static const float ACCENT_WIDTH = 0.012f;	// 左の印の幅（板の幅に対する割合）
static const float ACCENT_GAP = 0.008f;		// 左の印と帯の間の空き
static const float SCANLINE_COUNT = 14.0f;	// 走査線の本数
static const float SHINE_SPEED = 0.35f;		// 光の筋が流れる速さ（1 秒に板の幅の何倍）
static const float SHINE_WIDTH = 0.04f;		// 光の筋の幅
static const float3 BAND_COLOR = float3(0.05f, 0.06f, 0.19f);	// 帯の紺（#0d1030）
static const float BAND_ALPHA = 0.82f;							// 帯の濃さ
static const float3 ACCENT_COLOR = float3(1.0f, 0.17f, 0.84f);	// 左の印のマゼンタ（#ff2bd6）

cbuffer cb : register(b0){
	float4x4 mvp;		// モデルビュープロジェクション行列
	float4 mulColor;	// 乗算カラー（使わない）
};

// HudShape の ShapeData と同じ並び
cbuffer ShapeCb : register(b1){
	float4 shapeColor;	// 上下の線の色。a で全体を薄くする
	float value;		// 帯が出ている割合（0〜1。左から出す）
	float time;			// 経過時間（秒）。光の筋に使う
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

float4 PSMain(PSInput In) : SV_Target0
{
	// 斜めに切るため、高さに応じて横をずらした座標で考える（上ほど右へ）
	const float x = In.uv.x + (In.uv.y - 0.5f) * SLANT;
	const float left = SLANT * 0.5f;
	const float right = 1.0f - SLANT * 0.5f;

	// 左から出していく（出ていない所は描かない）
	const float revealRight = lerp(left, right, saturate(value));
	if (x < left || x > revealRight)
	{
		discard;
	}

	float3 color = BAND_COLOR;
	float alpha = BAND_ALPHA;

	// 左の印（マゼンタの縦棒）と、帯の間の空き
	if (x < left + ACCENT_WIDTH)
	{
		color = ACCENT_COLOR;
		alpha = 1.0f;
	}
	else if (x < left + ACCENT_WIDTH + ACCENT_GAP)
	{
		discard;
	}
	// 上下のシアンの線
	else if (In.uv.y < EDGE_LINE || In.uv.y > 1.0f - EDGE_LINE)
	{
		color = shapeColor.rgb;
		alpha = 1.0f;
	}
	else
	{
		// 走査線で少しだけ明暗をつける
		color *= 0.85f + 0.15f * step(0.5f, frac(In.uv.y * SCANLINE_COUNT));
		// 光の筋がときどき横切る
		const float shineX = frac(time * SHINE_SPEED) * 2.0f - 0.5f;
		const float shine = saturate(1.0f - abs(x - shineX) / SHINE_WIDTH);
		color += shapeColor.rgb * shine * 0.25f;
	}

	return float4(color, alpha * shapeColor.a);
}
