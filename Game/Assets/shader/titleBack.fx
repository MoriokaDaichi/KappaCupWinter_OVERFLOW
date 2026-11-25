/*!
 * @brief	タイトルの 3D の背景の板（title_back.tkm。端末と腕の後ろ）
 * 板の形には関係なく、画面の上の位置から色を決める（紺の地に、端末のうしろのぼんやりした光と、ゆっくり流れる横の走査線）
 */

static const float3 BACK_COLOR = float3(0.02f, 0.02f, 0.05f);	// 背景
static const float3 BACK_GLOW = float3(0.04f, 0.12f, 0.15f);	// 端末のうしろの、ぼんやりした光
static const float2 GLOW_CENTER = float2(0.5f, 0.52f);			// 光の中心（画面の割合。左上が 0,0）
static const float2 GLOW_RADIUS = float2(0.47f, 0.45f);			// 光の広がり（画面の割合）
static const float3 LINE_COLOR = float3(1.0f, 0.17f, 0.84f);	// 流れる線のマゼンタ
static const float LINE_COUNT = 9.0f;							// 流れる線の本数
static const float LINE_SPEED = 0.05f;							// 流れる速さ（1 秒に画面の高さの割合）

cbuffer ModelCb : register(b0)
{
	float4x4 mWorld;
	float4x4 mView;
	float4x4 mProj;
};

// TitleScene の BackConstant と同じ並び
cbuffer BackCb : register(b1)
{
	float time;		// 経過時間（秒）
	float3 pad;
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
	noperspective float2 screenUV : TEXCOORD0;	// 画面の上の位置（左上が 0,0）。画面の上でまっすぐ補間する
};

PSInput VSMain(VSInput In)
{
	PSInput Out;
	Out.pos = mul(mProj, mul(mView, mul(mWorld, In.pos)));
	const float2 ndc = Out.pos.xy / Out.pos.w;
	Out.screenUV = float2(ndc.x * 0.5f + 0.5f, 0.5f - ndc.y * 0.5f);
	return Out;
}

float4 PSMain(PSInput In) : SV_Target0
{
	const float2 uv = In.screenUV;
	const float2 g = (uv - GLOW_CENTER) / GLOW_RADIUS;
	float3 color = BACK_COLOR + BACK_GLOW * saturate(1.0f - dot(g, g));

	// ゆっくり上へ流れる、細いマゼンタの横線（電脳空間の気配）
	const float lines = frac(uv.y * LINE_COUNT + time * LINE_SPEED * LINE_COUNT);
	color += LINE_COLOR * 0.05f * smoothstep(0.97f, 1.0f, lines);

	return float4(color, 1.0f);
}
