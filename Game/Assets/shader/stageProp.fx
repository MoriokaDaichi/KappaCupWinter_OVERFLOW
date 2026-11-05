/*!
 * @brief	ステージの小物（ゲート・柱・ケーブル・機械）。暗い面に、発光マスクの模様が光る
 */

#include "stageCommon.h"

static const float3 BASE_COLOR = float3(0.010f, 0.012f, 0.040f);	// 面の色
static const float MASK_BRIGHTNESS = 1.0f;		// 模様の明るさ
static const float PULSE_BRIGHTNESS = 3.0f;		// 流れる光の明るさ（模様の明るさに掛ける）
static const float SIDE_SHADE = 0.6f;			// 横を向いた面の暗さ（形を見分けやすくする）

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
	return Out;
}

float4 PSMain(PSInput In) : SV_Target0
{
	const float3 normal = normalize(cross(ddx(In.worldPos), ddy(In.worldPos)));
	const float shade = lerp(SIDE_SHADE, 1.0f, abs(normal.y));
	const float3 lineColor = lerp(CYAN, MAGENTA, In.tint);
	const float mask = g_mask.Sample(g_sampler, In.uv).r;
	const float pulse = CalcPulse(In.worldPos.z, g_time);

	const float3 color = BASE_COLOR * shade + lineColor * mask * MASK_BRIGHTNESS * (1.0f + pulse * PULSE_BRIGHTNESS);
	return float4(ApplyFog(color, In.worldPos - g_cameraPos), 1.0f);
}
