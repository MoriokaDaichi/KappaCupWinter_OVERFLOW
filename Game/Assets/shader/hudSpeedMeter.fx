/*!
 * @brief	右下の 4 分円のスピードメーター（板の右下の角が中心。左が 0、上が振り切り）
 * 外の 4 分円と、速度の数字を囲む内側の丸い計器を、同じ作り（細い縁・縁の内側の細かい目盛り・5 本ごとに長い目盛り）で描く
 * value: 針の割合（0〜1）、flag: OVERDRIVE 中で 1
 */

static const float HALF_PI = 1.5707963f;
static const float TWO_PI = 6.2831853f;

// 外と内で共通の見た目（板の大きさに対する割合）
static const float RING_WIDTH = 0.006f;			// 縁の太さ（半分）
static const float TICK_WIDTH = 0.003f;			// 目盛りの太さ（半分）
static const float TICK_SHORT = 0.022f;			// 短い目盛りの長さ
static const float TICK_LONG = 0.040f;			// 長い目盛りの長さ
static const float LONG_EVERY = 5.0f;			// 何本ごとに長い目盛りにするか

// 外の 4 分円
static const float OUTER_RADIUS = 0.96f;		// 縁の半径
static const float OUTER_TICK_COUNT = 20.0f;	// 目盛りの数（長い目盛りで 4 つに区切る）
static const float NEEDLE_LENGTH = 0.90f;		// 針の長さ
static const float NEEDLE_WIDTH = 0.012f;		// 針の太さ
static const float PIVOT_RADIUS = 0.05f;		// 針の根元の丸の半径
static const float BACK_ALPHA = 0.55f;			// 背景（暗い 4 分円）の濃さ

// 内側の丸い計器（位置は SpeedMeter.h の SPEED_X_RATE・SPEED_Y_RATE と同じにする）
static const float2 DIAL_CENTER = float2(0.30f, 0.26f);	// 中心（右下の角から左・上への割合）
static const float DIAL_RADIUS = 0.17f;			// 縁の半径
static const float DIAL_TICK_COUNT = 40.0f;		// 目盛りの数
static const float DIAL_BACK_ALPHA = 0.85f;		// 丸の中の背景の濃さ（後ろの針がうっすら見える）

static const float3 BACK_COLOR = float3(0.03f, 0.03f, 0.10f);	// 背景の紺
static const float3 LINE_COLOR = float3(1.0f, 1.0f, 1.0f);		// 縁と目盛りの白
static const float3 OVERDRIVE_COLOR = float3(0.0f, 0.90f, 1.0f);// OVERDRIVE 中のシアン（#00e5ff）
static const float OVERDRIVE_BLINK = 8.0f;		// OVERDRIVE 中の明滅の速さ
static const float TICK_ALPHA = 0.8f;			// 目盛りの濃さ

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
	float value2;		// 使わない
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

/**
 * 縁の内側に並ぶ目盛りの濃さ
 * rate: 目盛りを並べる範囲の中の角度の割合（0〜1）、sweep: その範囲の角度（ラジアン）
 * 割合を目盛りの番号に直し、いちばん近い目盛りまでの弧の長さで太さを決める。LONG_EVERY 本ごとに長くする
 */
float Ticks(const float rate, const float count, const float sweep, const float r, const float radius)
{
	const float index = rate * count;
	const float distance = abs(frac(index + 0.5f) - 0.5f) / count * sweep * r;
	const float isLong = 1.0f - step(0.01f, frac(round(index) / LONG_EVERY));
	const float tickLength = lerp(TICK_SHORT, TICK_LONG, isLong);
	const float inRange = step(radius - tickLength, r) * step(r, radius);
	return Smooth(distance, TICK_WIDTH) * inRange;
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

	const float isOverdrive = step(0.5f, flag);
	const float blink = 0.75f + 0.25f * sin(time * OVERDRIVE_BLINK);
	const float4 lineColor = float4(lerp(LINE_COLOR, OVERDRIVE_COLOR * (1.0f + blink), isOverdrive), 1.0f);

	// 外の 4 分円: 背景・縁・目盛り
	float4 color = float4(BACK_COLOR, BACK_ALPHA * Smooth(r, OUTER_RADIUS));
	const float angleRate = saturate(atan2(q.y, q.x) / HALF_PI);	// 左を 0、上を 1 とした角度の割合
	const float ring = Smooth(abs(r - OUTER_RADIUS), RING_WIDTH);
	const float tick = Ticks(angleRate, OUTER_TICK_COUNT, HALF_PI, r, OUTER_RADIUS);
	color = lerp(color, lineColor, max(ring, tick * TICK_ALPHA));

	// 針
	const float needleAngle = value * HALF_PI;
	const float2 direction = float2(cos(needleAngle), sin(needleAngle));
	const float along = dot(q, direction);
	const float side = abs(q.x * direction.y - q.y * direction.x);
	const float needle = Smooth(side, NEEDLE_WIDTH) * step(0.0f, along) * step(along, NEEDLE_LENGTH);
	const float pivot = Smooth(r, PIVOT_RADIUS);
	color = lerp(color, float4(1.0f, 1.0f, 1.0f, 1.0f), max(needle, pivot));

	// 内側の丸い計器（速度の数字が読めるように、針より上に描く）
	const float2 d = q - DIAL_CENTER;
	const float dr = length(d);
	color = lerp(color, float4(BACK_COLOR, 1.0f), Smooth(dr, DIAL_RADIUS) * DIAL_BACK_ALPHA);
	const float dialRate = atan2(d.y, d.x) / TWO_PI + 0.5f;		// 一周を 0〜1 にした角度の割合
	const float dialRing = Smooth(abs(dr - DIAL_RADIUS), RING_WIDTH);
	const float dialTick = Ticks(dialRate, DIAL_TICK_COUNT, TWO_PI, dr, DIAL_RADIUS);
	color = lerp(color, lineColor, max(dialRing, dialTick * TICK_ALPHA));

	return color;
}
