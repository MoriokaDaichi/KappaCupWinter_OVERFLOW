/*!
 * @brief	タイトルの端末の画面（3D の端末の画面の場所に重ねる 2D の板）。黒に走査線と内側の光。押した後は白く光る
 * flag: 白く光る量（0〜1）
 */

static const float3 PANEL_COLOR = float3(0.015f, 0.03f, 0.05f);	// 画面の黒
static const float3 INNER_GLOW = float3(0.0f, 0.05f, 0.04f);		// 画面の内側の、ほのかな光
static const float SCANLINE_COUNT = 300.0f;							// 走査線の本数
static const float CORNER = 0.02f;									// 角の丸み（板の幅に対する割合）

cbuffer cb : register(b0){
	float4x4 mvp;		// モデルビュープロジェクション行列
	float4 mulColor;	// 乗算カラー（使わない）
};

// HudShape の ShapeData と同じ並び
cbuffer ShapeCb : register(b1){
	float4 shapeColor;	// 使わない
	float value;		// 使わない
	float time;			// 使わない
	float flag;			// 白く光る量（0〜1）
	float value2;		// 板の高さ÷幅（角を丸くするのに使う）
};

struct VSInput{
	float4 pos : POSITION;
	float2 uv  : TEXCOORD0;
};

struct PSInput{
	float4 pos : SV_POSITION;
	float2 uv  : TEXCOORD0;
};

PSInput VSMain(VSInput In)
{
	PSInput psIn;
	psIn.pos = mul(mvp, In.pos);
	psIn.uv = In.uv;
	return psIn;
}

float4 PSMain(PSInput In) : SV_Target0
{
	// 角を少し丸める
	const float aspect = max(value2, 0.01f);
	const float2 p = (In.uv - 0.5f) * float2(1.0f, aspect);
	const float2 q = abs(p) - (float2(0.5f, 0.5f * aspect) - CORNER);
	const float d = length(max(q, 0.0f)) + min(max(q.x, q.y), 0.0f) - CORNER;
	if (d > 0.0f)
	{
		discard;
	}

	const float scan = 0.9f + 0.1f * sin(In.uv.y * SCANLINE_COUNT * 3.14159f);
	const float2 edge = abs(In.uv - 0.5f) * 2.0f;
	const float inner = saturate(1.0f - pow(max(edge.x, edge.y), 6.0f));
	float3 color = PANEL_COLOR * scan + INNER_GLOW * inner;
	color = lerp(color, float3(1.0f, 1.0f, 1.0f), saturate(flag));
	return float4(color, 1.0f);
}
