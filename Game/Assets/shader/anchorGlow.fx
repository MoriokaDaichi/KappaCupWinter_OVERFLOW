/*!
 * @brief	アンカーの見た目。ライトに関係なく光り、ふちほど明るい
 */

#include "stageCommon.h"

static const float CENTER_BRIGHTNESS = 1.5f;	// 中心の明るさ
static const float RIM_BRIGHTNESS = 8.0f;		// ふちの明るさ（1 より大きいと Bloom で光る）
static const float RIM_SHARPNESS = 2.0f;		// ふちの光の細さ

cbuffer ModelCb : register(b0)
{
	float4x4 mWorld;
	float4x4 mView;
	float4x4 mProj;
};

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
	float3 viewNormal : NORMAL;
};

PSInput VSMain(VSInput In)
{
	PSInput Out;
	Out.pos = mul(mProj, mul(mView, mul(mWorld, In.pos)));
	Out.viewNormal = mul((float3x3)mView, mul((float3x3)mWorld, In.normal));
	return Out;
}

float4 PSMain(PSInput In) : SV_Target0
{
	const float rim = pow(1.0f - abs(normalize(In.viewNormal).z), RIM_SHARPNESS);
	return float4(CYAN * (CENTER_BRIGHTNESS + RIM_BRIGHTNESS * rim), 1.0f);
}
