/*!
 * @brief	タイトルから電脳空間へダイブするときの画面（企画書の絵: マゼンタの四角い通路を落ちていく主人公）
 * 端末の画面の大きさから通路が広がって画面いっぱいになり、四角い枠が手前へ流れてくる。真ん中を主人公の影が落ちていく
 * value: ダイブの進み（0〜1）
 */

static const float2 SCREEN_SIZE = float2(1920.0f, 1080.0f);		// 板の大きさ（UI の空間と同じ）
static const float ASPECT = 16.0f / 9.0f;						// 画面の幅÷高さ
static const float PANEL_SCALE = 0.56f;							// 端末の画面の、全体に対する大きさ（ここから広がる）
static const float RING_DENSITY = 1.6f;							// 枠の間隔（小さいほど広い）
static const float RING_SPEED = 2.2f;							// 枠が手前へ流れる速さ
static const float LINE_WIDTH = 3.0f;							// 主人公の輪郭の線の太さ
static const float3 MAGENTA = float3(1.0f, 0.17f, 0.84f);
static const float3 BACK_COLOR = float3(0.03f, 0.01f, 0.06f);
static const float3 BODY_COLOR = float3(0.12f, 0.13f, 0.26f);	// 主人公の紺
static const float3 BODY_LINE = float3(0.9f, 0.92f, 1.0f);		// 主人公の輪郭の白

cbuffer cb : register(b0){
	float4x4 mvp;		// モデルビュープロジェクション行列
	float4 mulColor;	// 乗算カラー（使わない）
};

// HudShape の ShapeData と同じ並び
cbuffer ShapeCb : register(b1){
	float4 shapeColor;	// 使わない
	float value;		// ダイブの進み（0〜1）
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

float Capsule(float2 p, float2 a, float2 b, float r)
{
	const float2 pa = p - a;
	const float2 ba = b - a;
	const float h = saturate(dot(pa, ba) / dot(ba, ba));
	return length(pa - ba * h) - r;
}

float2 Rotate(float2 p, float angle)
{
	const float s = sin(angle);
	const float c = cos(angle);
	return float2(c * p.x - s * p.y, s * p.x + c * p.y);
}

/** 落ちていく主人公の影までの距離（企画書の絵のポーズ。片腕を上げ、片足を曲げる） */
float Hacker(float2 p)
{
	float d = length(p - float2(0.0f, -100.0f)) - 30.0f;						// 頭
	d = min(d, Capsule(p, float2(0.0f, -55.0f), float2(12.0f, 40.0f), 36.0f));	// 胴
	d = min(d, Capsule(p, float2(-15.0f, -45.0f), float2(-105.0f, -20.0f), 15.0f));	// 左腕
	d = min(d, Capsule(p, float2(20.0f, -50.0f), float2(75.0f, -140.0f), 15.0f));	// 右腕（上げる）
	d = min(d, Capsule(p, float2(0.0f, 50.0f), float2(8.0f, 165.0f), 17.0f));		// 左足
	d = min(d, Capsule(p, float2(22.0f, 45.0f), float2(80.0f, 125.0f), 17.0f));		// 右足（曲げる）
	return d;
}

float4 PSMain(PSInput In) : SV_Target0
{
	const float2 pixel = In.uv * SCREEN_SIZE;
	const float2 c = (pixel - SCREEN_SIZE * 0.5f) / (SCREEN_SIZE.y * 0.5f);		// 真ん中が 0、上下の端が ±1
	// 四角い通路の「どれだけ外側か」（画面のふちで 1）
	const float r = max(abs(c.x) / ASPECT, abs(c.y));

	float3 color = BACK_COLOR;

	// 手前へ流れてくる四角い枠（外ほど明るい）
	const float v = -log2(max(r, 0.001f)) * RING_DENSITY + time * RING_SPEED;
	const float ring = smoothstep(0.42f, 0.5f, abs(frac(v) - 0.5f));
	const float depthFade = saturate(r * 1.4f);
	color += MAGENTA * ring * depthFade * 1.3f;

	// 四隅へ向かう斜めの線
	const float diagonal = 1.0f - smoothstep(0.0f, 0.006f, abs(abs(c.x) / ASPECT - abs(c.y)));
	color += MAGENTA * diagonal * depthFade;

	// 真ん中の奥の光
	color += MAGENTA * 0.25f * saturate(1.0f - r * 4.0f);

	// 落ちていく主人公（だんだん近づき、ゆらゆら回る）
	const float scale = lerp(0.55f, 1.1f, value);
	const float2 local = Rotate(pixel - SCREEN_SIZE * 0.5f, sin(time * 2.0f) * 0.25f) / scale;
	const float hacker = Hacker(local) * scale;
	const float fillRate = 1.0f - smoothstep(-1.0f, 1.0f, hacker);
	const float lineRate = 1.0f - smoothstep(LINE_WIDTH - 1.0f, LINE_WIDTH + 1.0f, abs(hacker));
	color = lerp(color, BODY_COLOR, fillRate);
	color = lerp(color, BODY_LINE, lineRate);

	// 出始めは端末の画面の大きさから広がる（画面に吸い込まれたように見せる）。外側は端末の紺
	const float open = lerp(PANEL_SCALE, 1.05f, smoothstep(0.0f, 0.25f, value));
	const float outside = smoothstep(open - 0.005f, open + 0.005f, r);
	color = lerp(color, float3(0.14f, 0.15f, 0.30f), outside);

	return float4(color, 1.0f);
}
