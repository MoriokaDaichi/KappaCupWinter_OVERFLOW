/*!
 * @brief	リザルトのランクの枠（企画書の絵: 紺の箱に金の枠）。value で白く光らせる（ランクが押された瞬間）
 */

static const float BORDER = 0.022f;		// 枠の太さ（板の幅に対する割合）
static const float CORNER = 0.08f;		// 角の丸み（板の幅に対する割合）
static const float GLOW = 0.03f;		// 枠のまわりの光の広がり
static const float3 FILL_COLOR = float3(0.05f, 0.06f, 0.18f);	// 中の紺
static const float FILL_ALPHA = 0.92f;

cbuffer cb : register(b0){
	float4x4 mvp;		// モデルビュープロジェクション行列
	float4 mulColor;	// 乗算カラー（使わない）
};

// HudShape の ShapeData と同じ並び
cbuffer ShapeCb : register(b1){
	float4 shapeColor;	// 枠の色（a で全体の濃さ）
	float value;		// 白く光らせる量（0〜1）
	float time;			// 使わない
	float flag;			// 使わない
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
	// 幅を 1 とした座標で、角の丸い四角までの距離を出す（外がプラス）
	const float aspect = max(value2, 0.01f);
	const float2 p = (In.uv - 0.5f) * float2(1.0f, aspect);
	const float2 half_ = float2(0.5f, 0.5f * aspect) - GLOW - CORNER;
	const float2 q = abs(p) - half_;
	const float d = length(max(q, 0.0f)) + min(max(q.x, q.y), 0.0f) - CORNER;

	const float border = 1.0f - smoothstep(0.0f, 0.003f, abs(d + BORDER * 0.5f) - BORDER * 0.5f);
	const float glow = exp(-max(d, 0.0f) / (GLOW * 0.4f)) * step(0.0f, d);
	const float inside = 1.0f - smoothstep(-0.002f, 0.0f, d);

	float3 color = FILL_COLOR;
	float alpha = inside * FILL_ALPHA;
	color = lerp(color, shapeColor.rgb, border);
	alpha = max(alpha, border);
	color = lerp(color, shapeColor.rgb, glow * (1.0f - inside));
	alpha = max(alpha, glow * 0.6f);

	// 押された瞬間の白い光
	color = lerp(color, float3(1.0f, 1.0f, 1.0f), saturate(value) * inside);

	if (alpha <= 0.0f)
	{
		discard;
	}
	return float4(color, alpha * shapeColor.a);
}
