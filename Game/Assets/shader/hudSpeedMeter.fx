/*!
 * @brief	右下の 4 分円のスピードメーター（板の右下の角が中心。左が 0、上が振り切り）
 * value: 針の割合（0〜1）、flag: OVERDRIVE 中で 1
 */

static const float HALF_PI = 1.5707963f;

static const float ARC_OUTER = 0.92f;			// 弧の外側の半径（板の大きさに対する割合）
static const float ARC_WIDTH = 0.10f;			// 弧の太さ
static const float TICK_COUNT = 4.0f;			// 目盛りで区切る数
static const float TICK_WIDTH = 0.012f;			// 目盛りの太さ（角度の割合）
static const float NEEDLE_LENGTH = 0.86f;		// 針の長さ
static const float NEEDLE_WIDTH = 0.012f;		// 針の太さ
static const float PIVOT_RADIUS = 0.05f;		// 針の根元の丸の半径
static const float BACK_ALPHA = 0.55f;			// メーターの背景（暗い 4 分円）の濃さ

static const float3 BACK_COLOR = float3(0.03f, 0.03f, 0.10f);	// 背景の紺
static const float3 EMPTY_COLOR = float3(0.22f, 0.22f, 0.42f);	// まだ満ちていない弧
static const float3 LOW_COLOR = float3(0.71f, 0.30f, 1.0f);		// 弧の根元の紫（#b44dff）
static const float3 HIGH_COLOR = float3(1.0f, 0.17f, 0.84f);	// 弧の先のマゼンタ（#ff2bd6）
static const float3 OVERDRIVE_COLOR = float3(0.0f, 0.90f, 1.0f);// OVERDRIVE 中のシアン（#00e5ff）
static const float OVERDRIVE_BLINK = 8.0f;		// OVERDRIVE 中の明滅の速さ

cbuffer cb : register(b0){
	float4x4 mvp;		// モデルビュープロジェクション行列
	float4 mulColor;	// 乗算カラー（使わない）
};

// HudShape の ShapeData と同じ並び
cbuffer ShapeCb : register(b1){
	float4 shapeColor;	// 使わない
	float value;		// 針の割合
	float time;			// 経過時間（秒）
	float flag;			// OVERDRIVE 中で 1
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

/** 境目を 1 ピクセルぶんぼかした「distance が width 以下」の濃さ */
float Smooth(const float distance, const float width)
{
	const float pixel = max(fwidth(distance), 0.0001f);
	return 1.0f - smoothstep(width - pixel, width + pixel, distance);
}

float4 PSMain(PSInput In) : SV_Target0
{
	// 右下の角を 0,0 にして、左と上をプラスにする
	const float2 q = float2(1.0f - In.uv.x, 1.0f - In.uv.y);
	const float r = length(q);
	if (r > 1.0f)
	{
		discard;
	}

	// 左を 0、上を 1 とした角度の割合
	const float angleRate = saturate(atan2(q.y, q.x) / HALF_PI);
	const float isOverdrive = step(0.5f, flag);
	const float blink = 0.75f + 0.25f * sin(time * OVERDRIVE_BLINK);

	float4 color = float4(BACK_COLOR, BACK_ALPHA * Smooth(r, 1.0f));

	// 弧（満ちたところは紫からマゼンタ、OVERDRIVE 中はシアンで明滅）
	const float arc = Smooth(abs(r - (ARC_OUTER - ARC_WIDTH * 0.5f)), ARC_WIDTH * 0.5f);
	const float filled = step(angleRate, value);
	float3 fillColor = lerp(LOW_COLOR, HIGH_COLOR, angleRate);
	fillColor = lerp(fillColor, OVERDRIVE_COLOR * (1.0f + blink), isOverdrive);
	const float3 arcColor = lerp(EMPTY_COLOR, fillColor, filled);
	color = lerp(color, float4(arcColor, 1.0f), arc);

	// 目盛り（弧の上だけ）
	const float tickDistance = abs(frac(angleRate * TICK_COUNT + 0.5f) - 0.5f) / TICK_COUNT;
	const float tick = Smooth(tickDistance, TICK_WIDTH) * arc;
	color = lerp(color, float4(1.0f, 1.0f, 1.0f, 1.0f), tick * 0.8f);

	// 針
	const float needleAngle = value * HALF_PI;
	const float2 direction = float2(cos(needleAngle), sin(needleAngle));
	const float along = dot(q, direction);
	const float side = abs(q.x * direction.y - q.y * direction.x);
	const float needle = Smooth(side, NEEDLE_WIDTH) * step(0.0f, along) * step(along, NEEDLE_LENGTH);
	const float pivot = Smooth(r, PIVOT_RADIUS);
	color = lerp(color, float4(1.0f, 1.0f, 1.0f, 1.0f), max(needle, pivot));

	return color;
}
