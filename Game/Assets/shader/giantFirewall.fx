/*!
 * @brief	巨大ファイアウォールの見た目（暗い赤の面に、赤く光る六角形の格子と「FIREWALL // ACCESS DENIED」の文字）
 * ライトに関係なく光る。遠くからでもゴールが見えるように、霧で消しきらない
 */

#include "stageCommon.h"

static const float3 WALL_RED = float3(1.0f, 0.08f, 0.12f);		// 線と文字の赤
static const float3 FACE_COLOR = float3(0.05f, 0.004f, 0.012f);	// 面の暗い赤

static const float HEX_SIZE = 260.0f;			// 六角形の大きさ（向かい合う辺の間の距離）
static const float HEX_LINE_WIDTH = 0.045f;		// 格子の線の太さ（六角形の大きさに対する割合）
static const float HEX_BRIGHTNESS = 3.0f;		// 格子の線の明るさ（1 より大きいと Bloom で光る）
static const float CELL_BLINK_RATE = 0.08f;		// 明滅するマスの割合
static const float CELL_BLINK_SPEED = 1.5f;		// マスの明滅の速さ
static const float CELL_BRIGHTNESS = 0.35f;		// 明滅するマスの明るさ

static const float SCAN_INTERVAL = 1800.0f;		// 上へ流れる走査線の間隔
static const float SCAN_SPEED = 700.0f;			// 走査線の速さ
static const float SCAN_SHARPNESS = 30.0f;		// 走査線の細さ
static const float SCAN_BRIGHTNESS = 2.0f;		// 走査線の明るさ

static const float EDGE_WIDTH = 30.0f;			// 壁のふちの線の太さ
static const float EDGE_BRIGHTNESS = 8.0f;		// 壁のふちの明るさ

static const float TEXT_WIDTH = 1800.0f;		// 文字の画像を貼る幅（通路の幅 2000 に収める）
static const float TEXT_HEIGHT = 225.0f;		// 文字の画像を貼る高さ（画像の縦横比 2048:256 に合わせる）
static const float TEXT_CENTER_HEIGHT = 1400.0f;// 文字の中心の、床からの高さ（近づいても画面に入る高さ）
static const float TEXT_BRIGHTNESS = 6.0f;		// 文字の明るさ
static const float TEXT_BLINK_SPEED = 2.0f;		// 文字の明滅の速さ

static const float FOG_RATE = 0.6f;				// 霧の濃さ（1 にすると遠くで消える。ゴールなので少し残す）

cbuffer ModelCb : register(b0)
{
	float4x4 mWorld;
	float4x4 mView;
	float4x4 mProj;
};

// GiantFirewall の Constant と同じ並び
cbuffer FirewallCb : register(b1)
{
	float4 g_center;		// 壁の中心
	float4 g_halfSize;		// 壁の大きさの半分
	float3 g_cameraPos;		// カメラの位置
	float g_time;			// 経過時間（秒）
};

Texture2D<float4> g_textTexture : register(t10);	// 文字（黒地に白）
SamplerState g_sampler : register(s0);

struct VSInput
{
	float4 pos : POSITION;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD0;
	float3 tangent : TANGENT;
	float3 biNormal : BINORMAL;
	int4 indices : BLENDINDICES0;
	float4 weights : BLENDWEIGHT0;
};

struct PSInput
{
	float4 pos : SV_POSITION;
	float3 worldPos : TEXCOORD0;
};

PSInput VSMain(VSInput In)
{
	PSInput Out;
	const float4 worldPos = mul(mWorld, In.pos);
	Out.worldPos = worldPos.xyz;
	Out.pos = mul(mProj, mul(mView, worldPos));
	return Out;
}

/** 2 次元の値から、0〜1 の決まった乱数を作る */
float Hash(const float2 value)
{
	return frac(sin(dot(value, float2(12.9898f, 78.233f))) * 43758.5453f);
}

/**
 * 六角形の格子。x に辺までの距離（0 が辺、0.5 が中心）、yz にマスの番号を返す
 * 六角形を 2 つずらして並べ、近いほうの中心を選ぶ
 */
float3 CalcHex(const float2 position)
{
	const float2 ratio = float2(1.0f, 1.7320508f);
	const float2 shift = ratio * 0.5f;
	const float2 p = position / HEX_SIZE;
	const float2 a = (frac(p / ratio) - 0.5f) * ratio;
	const float2 b = (frac((p - shift) / ratio) - 0.5f) * ratio;
	const float2 local = (dot(a, a) < dot(b, b)) ? a : b;
	const float2 v = abs(local);
	const float toCenter = max(dot(v, normalize(ratio)), v.x);
	// マスの中心は半マス単位の値になる。計算誤差でピクセルごとにぶれないよう、整数に丸める
	return float3(0.5f - toCenter, round((p - local) * 2.0f));
}

float4 PSMain(PSInput In) : SV_Target0
{
	// 正面（プレイヤーの来る -Z 側）と裏は XY、横の面は ZY に格子を張る
	const float3 normal = abs(normalize(cross(ddx(In.worldPos), ddy(In.worldPos))));
	const float2 facePos = (normal.z > normal.x) ? In.worldPos.xy : In.worldPos.zy;

	float3 color = FACE_COLOR;

	// 六角形の格子と、ところどころ明滅するマス
	const float3 hex = CalcHex(facePos);
	const float linePixel = max(fwidth(hex.x), 0.0001f);
	const float hexLine = 1.0f - smoothstep(HEX_LINE_WIDTH, HEX_LINE_WIDTH + linePixel, hex.x);
	color += WALL_RED * hexLine * HEX_BRIGHTNESS;
	const float cellSeed = Hash(hex.yz);
	const float blink = step(1.0f - CELL_BLINK_RATE, cellSeed) * (0.5f + 0.5f * sin(g_time * CELL_BLINK_SPEED + cellSeed * 40.0f));
	color += WALL_RED * blink * CELL_BRIGHTNESS;

	// 上へ流れる走査線
	const float scan = pow(frac((In.worldPos.y - g_time * SCAN_SPEED) / SCAN_INTERVAL), SCAN_SHARPNESS);
	color += WALL_RED * scan * SCAN_BRIGHTNESS * (0.3f + hexLine);

	// 壁のふち
	const float3 toEdge = g_halfSize.xyz - abs(In.worldPos - g_center.xyz);
	const float edgeDistance = (normal.z > normal.x) ? min(toEdge.x, toEdge.y) : min(toEdge.z, toEdge.y);
	color = lerp(color, WALL_RED * EDGE_BRIGHTNESS, 1.0f - smoothstep(EDGE_WIDTH, EDGE_WIDTH + fwidth(edgeDistance), edgeDistance));

	// 文字（正面だけ）
	if (normal.z > normal.x)
	{
		const float floorHeight = g_center.y - g_halfSize.y;
		// 画像は変換のときに上下を反転している（-vflip）ので、上ほど v を大きくして読めるようにする
		const float2 uv = float2(
			(In.worldPos.x - g_center.x) / TEXT_WIDTH + 0.5f,
			0.5f + (In.worldPos.y - floorHeight - TEXT_CENTER_HEIGHT) / TEXT_HEIGHT);
		if (all(uv >= 0.0f) && all(uv <= 1.0f))
		{
			const float text = g_textTexture.Sample(g_sampler, uv).r;
			const float textBlink = 0.75f + 0.25f * sin(g_time * TEXT_BLINK_SPEED);
			// 格子の線と重なると読みにくいので、文字の後ろは暗い帯にする
			color = lerp(FACE_COLOR, WALL_RED * TEXT_BRIGHTNESS * textBlink, text);
		}
	}

	// 遠くは少しだけ空の色に寄せる（消しきらない）
	const float3 toPixel = In.worldPos - g_cameraPos;
	const float fog = saturate(dot(toPixel, toPixel) / (FOG_DISTANCE * FOG_DISTANCE)) * FOG_RATE;
	return float4(lerp(color, CalcSkyColor(toPixel), fog), 1.0f);
}
