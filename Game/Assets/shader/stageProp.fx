/*!
 * @brief	ステージの小物（ゲート・柱・ケーブル・機械）。暗い面に、発光マスクの模様が光る
 */

#include "stageCommon.h"

static const float3 BASE_COLOR = float3(0.010f, 0.012f, 0.040f);	// 面の色
static const float MASK_BRIGHTNESS = 1.0f;		// 模様の明るさ
static const float PULSE_BRIGHTNESS = 3.0f;		// 流れる光の明るさ（模様の明るさに掛ける）
static const float SIDE_SHADE = 0.6f;			// 横を向いた面の暗さ（形を見分けやすくする）
static const float3 KEY_DIRECTION = float3(0.35f, 0.5f, -0.79f);	// 面取りや段差を見せる、弱い光の向き（光が来る方向）
static const float KEY_BRIGHTNESS = 1.6f;		// その光の強さ（面の色に掛ける）
static const float RIM_BRIGHTNESS = 0.10f;		// 輪郭にうっすら乗る色の強さ
static const float RIM_SHARPNESS = 3.0f;		// 輪郭の光の細さ

cbuffer ModelCb : register(b0)
{
	float4x4 mWorld;
	float4x4 mView;
	float4x4 mProj;
};

// StageProp の Constant と同じ並び
cbuffer PropCb : register(b1)
{
	float3 g_cameraPos;	// カメラの位置
	float g_time;		// 経過時間（秒）
};

// StageProp の Instance と同じ並び
struct Instance
{
	float4 positionYaw;	// xyz が位置。w が Y 軸の回転（ラジアン）
	float4 scaleTint;	// xyz が大きさの倍率。w が色（0 でシアン、1 でマゼンタ）
};

StructuredBuffer<Instance> g_instances : register(t10);
Texture2D<float4> g_mask : register(t0);	// 発光マスク（モデルの色のテクスチャの枠に入っている）
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
	nointerpolation float tint : TEXCOORD2;
	float3 normal : TEXCOORD3;
};

PSInput VSMain(VSInput In)
{
	const Instance instance = g_instances[In.instanceId];

	// mWorld はモデルの Z 上向きを Y 上向きに直す回転だけ（Model::Init で単位の位置と大きさにしてある）
	const float3 local = mul(mWorld, float4(In.pos.xyz, 1.0f)).xyz * instance.scaleTint.xyz;
	const float s = sin(instance.positionYaw.w);
	const float c = cos(instance.positionYaw.w);
	const float3 rotated = float3(local.x * c + local.z * s, local.y, -local.x * s + local.z * c);

	PSInput Out;
	Out.worldPos = instance.positionYaw.xyz + rotated;
	Out.pos = mul(mProj, mul(mView, float4(Out.worldPos, 1.0f)));
	Out.uv = In.uv;
	Out.tint = instance.scaleTint.w;

	// 法線も同じように回す。倍率が軸ごとに違うので、倍率で割ってから向きをそろえる
	const float3 localNormal = mul(mWorld, float4(In.normal, 0.0f)).xyz / instance.scaleTint.xyz;
	Out.normal = float3(localNormal.x * c + localNormal.z * s, localNormal.y, -localNormal.x * s + localNormal.z * c);
	return Out;
}

float4 PSMain(PSInput In) : SV_Target0
{
	// 頂点の法線を使う（面取りがなめらかに見える）。倍率が負の小物でも同じ見た目になるよう、向きの正負は見ない
	const float3 normal = normalize(In.normal);
	const float3 toCamera = normalize(g_cameraPos - In.worldPos);
	const float key = abs(dot(normal, normalize(KEY_DIRECTION)));
	const float shade = lerp(SIDE_SHADE, 1.0f, abs(normal.y)) * (1.0f + key * KEY_BRIGHTNESS);
	const float rim = pow(1.0f - abs(dot(normal, toCamera)), RIM_SHARPNESS);
	const float3 lineColor = lerp(CYAN, MAGENTA, In.tint);
	const float mask = g_mask.Sample(g_sampler, In.uv).r;
	const float pulse = CalcPulse(In.worldPos.z, g_time);

	const float3 color = BASE_COLOR * shade + lineColor * rim * RIM_BRIGHTNESS
		+ lineColor * mask * MASK_BRIGHTNESS * (1.0f + pulse * PULSE_BRIGHTNESS);
	return float4(ApplyFog(color, In.worldPos - g_cameraPos), 1.0f);
}
