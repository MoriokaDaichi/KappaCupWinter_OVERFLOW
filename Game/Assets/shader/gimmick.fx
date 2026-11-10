/*!
 * @brief	ギミック（ダッシュ板・電気柵・レーザーの壁）。暗い本体に、マスクの場所が光る
 * マスク: R=光る所 G=ダッシュ板の矢印の場所 B=黄色と黒の警告の縞
 */

#include "stageCommon.h"

static const float KIND_DASH_PAD = 0.0f;	// GimmickView::Kind と同じ値
static const float KIND_FENCE = 1.0f;
static const float KIND_LASER = 2.0f;

static const float3 BASE_COLOR = float3(0.012f, 0.014f, 0.040f);	// 本体の色
static const float3 HAZARD_COLOR = float3(0.55f, 0.42f, 0.02f);		// 警告の縞の黄色
static const float3 FENCE_COLOR = float3(1.0f, 0.80f, 0.10f);		// 電気の黄色
static const float3 LASER_COLOR = float3(1.0f, 0.06f, 0.04f);		// レーザーの赤
static const float SIDE_SHADE = 0.6f;			// 横を向いた面の暗さ
static const float3 KEY_DIRECTION = float3(0.35f, 0.5f, -0.79f);	// 形を見せる弱い光の向き（光が来る方向）
static const float KEY_BRIGHTNESS = 1.6f;		// その光の強さ

static const float DASH_PAD_GLOW = 3.0f;		// ダッシュ板のレールの明るさ（1 より大きいと Bloom で光る）
static const float CHEVRON_INTERVAL = 160.0f;	// 矢印の間隔
static const float CHEVRON_SLOPE = 0.5f;		// 矢印の角度（横に 1 進むと、前に何進むか）
static const float CHEVRON_WIDTH = 0.35f;		// 矢印の太さ（間隔に対する割合）
static const float CHEVRON_SPEED = 2.5f;		// 矢印が流れる速さ（1 秒に何個分）
static const float CHEVRON_GLOW = 4.0f;			// 矢印の明るさ
static const float PAD_FLOOR_GLOW = 0.15f;		// 矢印のない所の、うっすらした明るさ

static const float FENCE_GLOW = 3.0f;			// 電線の明るさ
static const float FLICKER_DEPTH = 0.45f;		// ちらつきの強さ（0 でちらつかない）
static const float SPARK_INTERVAL = 500.0f;		// 電線を走る火花の間隔
static const float SPARK_SPEED = 2.5f;			// 火花の速さ（1 秒に何個分）
static const float SPARK_SHARPNESS = 30.0f;		// 火花の短さ
static const float SPARK_GLOW = 6.0f;			// 火花の明るさ

static const float LASER_GLOW = 6.0f;			// ビームの明るさ
static const float LASER_SHIMMER = 0.15f;		// ビームのゆらぎの強さ

cbuffer ModelCb : register(b0)
{
	float4x4 mWorld;
	float4x4 mView;
	float4x4 mProj;
};

// GimmickView の Constant と同じ並び
cbuffer GimmickCb : register(b1)
{
	float3 g_cameraPos;		// カメラの位置
	float g_time;			// 経過時間（秒）
	float g_brightness;		// 光る所の明るさの倍率
	float g_kind;			// 光り方
	float2 g_padding;
};

// GimmickView の Instance と同じ並び
struct Instance
{
	float4 position;	// xyz が底の中心
	float4 scale;		// xyz が倍率
};

StructuredBuffer<Instance> g_instances : register(t10);
Texture2D<float4> g_mask : register(t0);	// マスク（モデルの色のテクスチャの枠に入っている）
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
	uint instanceId : SV_InstanceID;
};

struct PSInput
{
	float4 pos : SV_POSITION;
	float3 worldPos : TEXCOORD0;
	float2 uv : TEXCOORD1;
	float3 normal : TEXCOORD2;
	nointerpolation float3 center : TEXCOORD3;
};

PSInput VSMain(VSInput In)
{
	const Instance instance = g_instances[In.instanceId];

	// mWorld はモデルの Z 上向きを Y 上向きに直す回転だけ（Model::Init で単位の位置と大きさにしてある）
	PSInput Out;
	Out.worldPos = instance.position.xyz + mul(mWorld, float4(In.pos.xyz, 1.0f)).xyz * instance.scale.xyz;
	Out.pos = mul(mProj, mul(mView, float4(Out.worldPos, 1.0f)));
	Out.uv = In.uv;
	Out.normal = mul(mWorld, float4(In.normal, 0.0f)).xyz / instance.scale.xyz;
	Out.center = instance.position.xyz;
	return Out;
}

/** ダッシュ板の矢印（0〜1）。板の真ん中を先頭にした「く」の字が、前（+Z）へ流れる */
float CalcChevron(const float3 worldPos, const float3 center)
{
	const float along = (worldPos.z - center.z + abs(worldPos.x - center.x) * CHEVRON_SLOPE) / CHEVRON_INTERVAL;
	return step(frac(along - g_time * CHEVRON_SPEED), CHEVRON_WIDTH);
}

/** 電気のちらつき（0〜1 くらい）と、電線を走る火花 */
float CalcElectric(const float3 worldPos)
{
	const float flicker = 1.0f - FLICKER_DEPTH * (0.5f + 0.5f * sin(g_time * 37.0f + worldPos.x * 0.05f) * sin(g_time * 23.0f - worldPos.x * 0.031f));
	const float spark = pow(frac(worldPos.x / SPARK_INTERVAL - g_time * SPARK_SPEED), SPARK_SHARPNESS) * SPARK_GLOW;
	return flicker * FENCE_GLOW + spark;
}

float4 PSMain(PSInput In) : SV_Target0
{
	const float3 normal = normalize(In.normal);
	const float key = abs(dot(normal, normalize(KEY_DIRECTION)));
	const float shade = lerp(SIDE_SHADE, 1.0f, abs(normal.y)) * (1.0f + key * KEY_BRIGHTNESS);
	const float3 mask = g_mask.Sample(g_sampler, In.uv).rgb;

	float3 color = BASE_COLOR * shade + HAZARD_COLOR * mask.b * shade;

	if (g_kind == KIND_DASH_PAD)
	{
		color += CYAN * mask.r * DASH_PAD_GLOW * g_brightness;
		const float chevron = CalcChevron(In.worldPos, In.center);
		color += CYAN * mask.g * lerp(PAD_FLOOR_GLOW, CHEVRON_GLOW, chevron) * g_brightness;
	}
	else if (g_kind == KIND_FENCE)
	{
		color += FENCE_COLOR * mask.r * CalcElectric(In.worldPos) * g_brightness;
	}
	else
	{
		const float shimmer = 1.0f + LASER_SHIMMER * sin(g_time * 50.0f + In.worldPos.x * 0.02f);
		color += LASER_COLOR * mask.r * LASER_GLOW * shimmer * g_brightness;
	}

	return float4(ApplyFog(color, In.worldPos - g_cameraPos), 1.0f);
}
