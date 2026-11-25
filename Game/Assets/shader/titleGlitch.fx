/*!
 * @brief	タイトルのロゴに重ねるグリッチの線（企画書の絵: ロゴを横切るマゼンタとシアンの帯）
 * ときどき帯の位置が跳び、細い帯が増える。value: 乱れの強さ（0 でふだん、1 で強く乱れている）
 */

static const float3 MAGENTA = float3(1.0f, 0.17f, 0.84f);
static const float3 CYAN = float3(0.0f, 0.9f, 1.0f);
static const float BAR_HEIGHT = 0.035f;		// 帯の太さ（板の高さに対する割合）

cbuffer cb : register(b0){
	float4x4 mvp;		// モデルビュープロジェクション行列
	float4 mulColor;	// 乗算カラー（使わない）
};

// HudShape の ShapeData と同じ並び
cbuffer ShapeCb : register(b1){
	float4 shapeColor;	// 使わない
	float value;		// 乱れの強さ（0〜1）
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

/** 0〜1 の疑似乱数 */
float Hash(float n)
{
	return frac(sin(n * 12.9898f) * 43758.5453f);
}

/** 左端 x0・右端 x1・高さ y の帯なら 1 */
float Bar(float2 uv, float x0, float x1, float y)
{
	return step(x0, uv.x) * step(uv.x, x1) * step(abs(uv.y - y), BAR_HEIGHT * 0.5f);
}

float4 PSMain(PSInput In) : SV_Target0
{
	const float2 uv = In.uv;
	// ふだんは少しだけ揺れる。乱れているときは、帯の位置が 1 秒に 30 回跳ぶ
	const float tick = floor(time * 30.0f);
	const float jump = value * (Hash(tick) - 0.5f) * 0.3f;

	const float magenta = Bar(uv, 0.05f + jump, 0.40f + jump, 0.40f + 0.01f * sin(time * 2.0f));
	const float cyan = Bar(uv, 0.56f - jump, 0.95f - jump, 0.58f + 0.01f * sin(time * 2.6f));

	// 乱れているときだけ出る細い帯
	const float extraY = Hash(tick * 1.7f);
	const float extraX = Hash(tick * 2.3f);
	const float extra = value * Bar(uv, extraX * 0.6f, extraX * 0.6f + 0.35f, extraY) * step(0.4f, Hash(tick * 3.1f));

	const float3 color = MAGENTA * magenta + CYAN * cyan + lerp(CYAN, MAGENTA, Hash(tick)) * extra;
	const float alpha = saturate(magenta + cyan + extra) * 0.85f;
	if (alpha <= 0.0f)
	{
		discard;
	}
	return float4(color, alpha);
}
