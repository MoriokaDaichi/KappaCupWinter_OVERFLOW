/*!
 * @brief	敵が狙っている間に出す予告線。ライトに関係なく赤く光り、細かくちらつく
 * 加算で描き、カメラの近くほど薄くする（線はプレイヤーを通り抜けてカメラの前まで来るので、手前が太い帯になって画面をふさがないように）
 */

static const float3 WARNING_RED = float3(1.0f, 0.06f, 0.10f);	// 予告線の赤
static const float FLICKER_SPEED = 40.0f;						// ちらつきの速さ（ラジアン / 秒）
static const float FLICKER_DEPTH = 0.3f;						// ちらつきで暗くなる割合（0 でちらつかない）
static const float NEAR_FADE_START = 50.0f;						// カメラからこの距離より近いと見えない
static const float NEAR_FADE_RANGE = 200.0f;					// NEAR_FADE_START からこの距離をかけて、元の明るさになる（カメラはプレイヤーの 300 後ろなので、プレイヤーの所では元の明るさ）

cbuffer ModelCb : register(b0)
{
	float4x4 mWorld;
	float4x4 mView;
	float4x4 mProj;
};

// WarningLine の Constant と同じ並び
cbuffer WarningLineCb : register(b1)
{
	float3 cameraPos;	// カメラの位置
	float brightness;	// 光の強さ
	float time;			// 出始めからの秒数
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
	float3 worldPos : TEXCOORD0;
};

PSInput VSMain(VSInput In)
{
	PSInput Out;
	const float4 worldPos = mul(mWorld, In.pos);
	Out.pos = mul(mProj, mul(mView, worldPos));
	Out.worldPos = worldPos.xyz;
	return Out;
}

float4 PSMain(PSInput In) : SV_Target0
{
	const float flicker = 1.0f - FLICKER_DEPTH * (0.5f + 0.5f * sin(time * FLICKER_SPEED));
	const float nearFade = saturate((distance(In.worldPos, cameraPos) - NEAR_FADE_START) / NEAR_FADE_RANGE);
	return float4(WARNING_RED * brightness * flicker * nearFade, 1.0f);
}
