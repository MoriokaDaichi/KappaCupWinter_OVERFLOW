/*!
 * @brief	タイトルの絵（企画書の「スタート」の絵）。手に持った携帯端末と、画面に触れる親指を描く
 * 画面の中の文字（コード・ロゴ・PRESS START）は TitleScene が HudText で上に重ねる
 * value: 親指で押し込んだ量（0〜1）。flag: 画面が白く光る量（0〜1。押した後に画面へ吸い込まれる前）
 */

static const float2 SCREEN_SIZE = float2(1920.0f, 1080.0f);		// 板の大きさ（UI の空間と同じ）
static const float2 DEVICE_CENTER = float2(960.0f, 520.0f);		// 端末の中心（ピクセル。左上が 0,0）
static const float2 DEVICE_HALF = float2(590.0f, 340.0f);		// 端末の大きさの半分
static const float DEVICE_RADIUS = 44.0f;						// 端末の角の丸み
static const float2 PANEL_HALF = float2(545.0f, 300.0f);		// 画面の大きさの半分
static const float PANEL_RADIUS = 20.0f;						// 画面の角の丸み
static const float LINE_WIDTH = 3.0f;							// 輪郭の線の太さ
static const float2 THUMB_TIP = float2(1330.0f, 690.0f);		// 親指の先（押していないとき）
static const float2 THUMB_BASE = float2(1780.0f, 1180.0f);		// 親指の付け根（画面の外）
static const float THUMB_RADIUS = 56.0f;						// 親指の太さの半分
static const float RING_RADIUS = 60.0f;							// 親指のまわりの輪の大きさ
static const float3 BACK_COLOR = float3(0.02f, 0.02f, 0.05f);	// 背景
static const float3 BACK_GLOW = float3(0.04f, 0.12f, 0.15f);	// 端末のうしろの、ぼんやりした光
static const float3 BODY_COLOR = float3(0.14f, 0.15f, 0.30f);	// 端末と手の紺
static const float3 BODY_LINE = float3(0.55f, 0.58f, 0.85f);	// 端末のふちの線
static const float3 HAND_LINE = float3(0.85f, 0.88f, 1.0f);		// 手の輪郭の線
static const float3 PANEL_COLOR = float3(0.015f, 0.03f, 0.05f);	// 画面の黒
static const float3 CYAN = float3(0.0f, 0.9f, 1.0f);

cbuffer cb : register(b0){
	float4x4 mvp;		// モデルビュープロジェクション行列
	float4 mulColor;	// 乗算カラー（使わない）
};

// HudShape の ShapeData と同じ並び
cbuffer ShapeCb : register(b1){
	float4 shapeColor;	// 使わない
	float value;		// 親指で押し込んだ量（0〜1）
	float time;			// 経過時間（秒）
	float flag;			// 画面が白く光る量（0〜1）
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

/** 角の丸い四角までの距離（中がマイナス） */
float RoundBox(float2 p, float2 half_, float r)
{
	const float2 q = abs(p) - half_ + r;
	return length(max(q, 0.0f)) + min(max(q.x, q.y), 0.0f) - r;
}

/** a から b までの太さ r の棒までの距離（中がマイナス） */
float Capsule(float2 p, float2 a, float2 b, float r)
{
	const float2 pa = p - a;
	const float2 ba = b - a;
	const float h = saturate(dot(pa, ba) / dot(ba, ba));
	return length(pa - ba * h) - r;
}

/** p を角度 angle（ラジアン）だけ回す */
float2 Rotate(float2 p, float angle)
{
	const float s = sin(angle);
	const float c = cos(angle);
	return float2(c * p.x - s * p.y, s * p.x + c * p.y);
}

/** 距離 d の形を、塗りと輪郭の線で重ねる */
void Layer(inout float3 color, float d, float3 fill, float3 line_)
{
	const float fillRate = 1.0f - smoothstep(-1.0f, 1.0f, d);
	const float lineRate = 1.0f - smoothstep(LINE_WIDTH - 1.0f, LINE_WIDTH + 1.0f, abs(d));
	color = lerp(color, fill, fillRate);
	color = lerp(color, line_, lineRate);
}

float4 PSMain(PSInput In) : SV_Target0
{
	const float2 p = In.uv * SCREEN_SIZE;

	// 背景と、端末のうしろの光
	const float2 glowPos = (p - float2(960.0f, 560.0f)) / float2(900.0f, 480.0f);
	float3 color = BACK_COLOR + BACK_GLOW * saturate(1.0f - dot(glowPos, glowPos));

	// 端末の体
	const float2 local = p - DEVICE_CENTER;
	Layer(color, RoundBox(local, DEVICE_HALF, DEVICE_RADIUS), BODY_COLOR, BODY_LINE);

	// 画面（黒に走査線。押した後は白く光る）
	const float panel = RoundBox(local, PANEL_HALF, PANEL_RADIUS);
	if (panel < 0.0f)
	{
		const float scan = 0.9f + 0.1f * sin(p.y * 1.6f);
		const float2 edge = abs(local) / PANEL_HALF;
		const float inner = saturate(1.0f - pow(max(edge.x, edge.y), 6.0f));
		float3 panelColor = PANEL_COLOR * scan + float3(0.0f, 0.05f, 0.04f) * inner;
		panelColor = lerp(panelColor, float3(1.0f, 1.0f, 1.0f), saturate(flag));
		color = panelColor;
	}
	// 画面のふちの細い線
	color = lerp(color, BODY_LINE * 0.6f, 1.0f - smoothstep(0.5f, 1.5f, abs(panel)));

	// 左手の指（端末の左のふちをつかむ 4 本）
	for (int i = 0; i < 4; i++)
	{
		const float y = 300.0f + 95.0f * i;
		Layer(color, Capsule(p, float2(320.0f, y), float2(430.0f, y + 6.0f), 38.0f), BODY_COLOR, HAND_LINE);
	}
	// 左手のてのひら（左下から）
	const float2 palm = Rotate(p - float2(300.0f, 930.0f), 0.45f);
	Layer(color, RoundBox(palm, float2(160.0f, 240.0f), 70.0f), BODY_COLOR, HAND_LINE);

	// 右手（右下から）と親指。押すと親指の先が少し画面へ沈む
	Layer(color, Capsule(p, float2(1950.0f, 1300.0f), float2(1720.0f, 1080.0f), 190.0f), BODY_COLOR, HAND_LINE);
	const float2 tip = THUMB_TIP + float2(-10.0f, 14.0f) * value;
	Layer(color, Capsule(p, THUMB_BASE, tip, THUMB_RADIUS), BODY_COLOR, HAND_LINE);
	// 爪
	const float2 thumbDir = normalize(tip - THUMB_BASE);
	const float nail = Capsule(p, tip - thumbDir * 45.0f, tip - thumbDir * 12.0f, 26.0f);
	color = lerp(color, HAND_LINE, 1.0f - smoothstep(LINE_WIDTH - 1.0f, LINE_WIDTH + 1.0f, abs(nail)));

	// 親指のまわりの輪。ふだんはゆっくり脈打ち、押すと外へ広がって消える
	const float idleRadius = RING_RADIUS + 6.0f * sin(time * 3.0f);
	const float radius = lerp(idleRadius, RING_RADIUS + 140.0f, value);
	const float ringAlpha = lerp(0.7f, 0.0f, value);
	const float ring = 1.0f - smoothstep(1.0f, 3.0f, abs(length(p - tip) - radius));
	color = lerp(color, CYAN, ring * ringAlpha);

	return float4(color, 1.0f);
}
