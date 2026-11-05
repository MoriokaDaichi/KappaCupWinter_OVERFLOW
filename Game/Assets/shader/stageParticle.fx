/*!
 * @brief	カメラのまわりを舞うデータの粒。位置はシェーダーの中で決めるので、CPU の計算はいらない
 * 箱のモデルの XY だけを使って板にする。カリングで表か裏の片方だけが残り、横の面はつぶれて見えない
 */

#include "stageCommon.h"

static const float MODEL_HALF_SIZE = 50.0f;		// 箱のモデルの 1 辺の半分
static const float AREA_SIZE = 7000.0f;			// 粒が舞う範囲（カメラを中心とした立方体の 1 辺）
static const float SIZE_MIN = 2.0f;				// 粒の大きさの半分の最小
static const float SIZE_RANGE = 4.0f;			// 粒の大きさの半分のばらつき
static const float RISE_SPEED = 60.0f;			// 上がる速さ
static const float DRIFT_SPEED = 200.0f;		// 進む向き（+Z）へ流れる速さ
static const float BRIGHTNESS = 1.5f;			// 明るさ
static const float BLINK_SPEED = 4.0f;			// 瞬きの速さ
static const float NEAR_FADE = 300.0f;			// カメラの近くで消える距離（画面を覆わないように）

cbuffer ModelCb : register(b0)
{
	float4x4 mWorld;
	float4x4 mView;
	float4x4 mProj;
};

// StageParticle の Constant と同じ並び
cbuffer ParticleCb : register(b1)
{
	float3 g_cameraPos;	// カメラの位置
	float g_time;		// 経過時間（秒）
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
	uint instanceId : SV_InstanceID;
};

struct PSInput
{
	float4 pos : SV_POSITION;
	float3 color : COLOR0;
};

/** 番号から 0〜1 の決まった乱数を作る */
float Hash(const float value)
{
	return frac(sin(value * 12.9898f) * 43758.5453f);
}

PSInput VSMain(VSInput In)
{
	const float id = (float)In.instanceId + 1.0f;
	const float3 seed = float3(Hash(id), Hash(id + 0.37f), Hash(id + 0.71f));

	// 範囲の中で繰り返すので、カメラが動いても粒が尽きない
	const float3 moved = seed * AREA_SIZE + float3(0.0f, RISE_SPEED, DRIFT_SPEED) * g_time - g_cameraPos;
	const float3 offset = (frac(moved / AREA_SIZE) - 0.5f) * AREA_SIZE;
	const float distanceFade = saturate((AREA_SIZE * 0.5f - length(offset)) / (AREA_SIZE * 0.25f));
	const float nearFade = saturate(length(offset) / NEAR_FADE);
	const float blink = 0.5f + 0.5f * sin(g_time * BLINK_SPEED + id);

	float4 viewPos = mul(mView, float4(g_cameraPos + offset, 1.0f));
	viewPos.xy += In.pos.xy / MODEL_HALF_SIZE * (SIZE_MIN + SIZE_RANGE * seed.x);

	PSInput Out;
	Out.pos = mul(mProj, viewPos);
	Out.color = ((seed.y < 0.5f) ? CYAN : MAGENTA) * distanceFade * nearFade * blink * BRIGHTNESS;
	return Out;
}

float4 PSMain(PSInput In) : SV_Target0
{
	return float4(In.color, 1.0f);
}
