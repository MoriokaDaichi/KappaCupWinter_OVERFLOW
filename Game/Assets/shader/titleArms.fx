/*!
 * @brief	タイトルの主人公の腕と手（title_arms.tkm）。テクスチャの色に、上からの光と、ふちのシアンの光を足す
 * 腕は黒い手袋と袖なので、ふちを光らせて紺の背景に溶けないようにする（敵の輪郭線と同じ考え方）
 */

static const float3 LIGHT_DIRECTION = float3(0.3f, 0.85f, -0.45f);	// 光が来る向き（ワールド。上と少し手前から）
static const float AMBIENT = 0.35f;									// どの向きにも当たる光
static const float DIFFUSE = 1.1f;									// 光の向きの面に当たる光
static const float3 RIM_COLOR = float3(0.0f, 0.9f, 1.0f);			// ふちの光のシアン
static const float RIM_BRIGHTNESS = 0.35f;						// ふちの明るさ（強いと手が光って見えるので控えめ）
static const float RIM_SHARPNESS = 4.0f;							// ふちの光の細さ

cbuffer ModelCb : register(b0)
{
	float4x4 mWorld;
	float4x4 mView;
	float4x4 mProj;
};

Texture2D<float4> g_albedo : register(t0);	// 色のテクスチャ（主人公と同じ body_diffuse）
sampler g_sampler : register(s0);

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
	float3 worldNormal : NORMAL;
	float3 viewNormal : TEXCOORD1;
	float2 uv : TEXCOORD0;
};

PSInput VSMain(VSInput In)
{
	PSInput Out;
	Out.pos = mul(mProj, mul(mView, mul(mWorld, In.pos)));
	Out.worldNormal = mul((float3x3)mWorld, In.normal);
	Out.viewNormal = mul((float3x3)mView, Out.worldNormal);
	Out.uv = In.uv;
	return Out;
}

float4 PSMain(PSInput In) : SV_Target0
{
	const float3 albedo = g_albedo.Sample(g_sampler, In.uv).rgb;
	const float3 n = normalize(In.worldNormal);
	const float light = AMBIENT + DIFFUSE * saturate(dot(n, normalize(LIGHT_DIRECTION)));
	const float rim = pow(1.0f - abs(normalize(In.viewNormal).z), RIM_SHARPNESS);
	return float4(albedo * light + RIM_COLOR * RIM_BRIGHTNESS * rim, 1.0f);
}
