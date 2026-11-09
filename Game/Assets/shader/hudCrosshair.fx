/*!
 * @brief	画面の真ん中の照準（真ん中を空けた 4 本の短い線）
 */

static const float GAP = 0.35f;			// 真ん中の空き（板の半分に対する割合）
static const float LINE_WIDTH = 0.07f;	// 線の太さの半分

cbuffer cb : register(b0){
	float4x4 mvp;		// モデルビュープロジェクション行列
	float4 mulColor;	// 乗算カラー（使わない）
};

// HudShape の ShapeData と同じ並び
cbuffer ShapeCb : register(b1){
	float4 shapeColor;	// 線の色
	float value;		// 使わない
	float time;			// 使わない
	float flag;			// 使わない
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
	const float2 p = abs(In.uv * 2.0f - 1.0f);
	const float horizontal = step(p.y, LINE_WIDTH) * step(GAP, p.x);
	const float vertical = step(p.x, LINE_WIDTH) * step(GAP, p.y);
	const float alpha = max(horizontal, vertical);
	if (alpha <= 0.0f)
	{
		discard;
	}
	return float4(shapeColor.rgb, shapeColor.a * alpha);
}
