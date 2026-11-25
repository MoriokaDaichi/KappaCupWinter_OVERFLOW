/*!
 * @brief	タイトルの 3D の端末（title_tablet.tkm）。紺の体に、ふちほど強く光るシアンの縁を付ける（ゲームのステージと同じネオンの見た目）
 * 画面の中身は TitleScene が 2D で上に重ねる
 */

static const float3 BODY_COLOR = float3(0.012f, 0.014f, 0.035f);	// 体の紺（自動露出で明るく持ち上がるので、かなり暗くしておく）
static const float3 RIM_COLOR = float3(0.0f, 0.9f, 1.0f);		// ふちの光のシアン
static const float RIM_BRIGHTNESS = 8.0f;						// ふちの明るさ（1 より大きいと Bloom で光る）
static const float RIM_SHARPNESS = 2.0f;						// ふちの光の細さ
static const float FRONT_LIGHT = 0.008f;							// 正面の面を少しだけ明るくする量（形が分かるように）

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
	const float3 n = normalize(In.viewNormal);
	// カメラに向いている面ほど 0、横を向いている面（角を丸めたふち）ほど 1
	const float rim = pow(1.0f - abs(n.z), RIM_SHARPNESS);
	const float3 color = BODY_COLOR + FRONT_LIGHT * abs(n.z) + RIM_COLOR * RIM_BRIGHTNESS * rim;
	return float4(color, 1.0f);
}
