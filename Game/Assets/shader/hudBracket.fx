/*!
 * @brief	ロックしたアンカーを囲む角カッコ（四隅の L 字）
 */

static const float LINE_WIDTH = 0.08f;		// 線の太さ（板の半分に対する割合）
static const float CORNER_LENGTH = 0.35f;	// L 字の長さ
static const float BLINK_SPEED = 6.0f;		// 明滅の速さ

cbuffer cb : register(b0){
	float4x4 mvp;		// モデルビュープロジェクション行列
	float4 mulColor;	// 乗算カラー（使わない）
};

// HudShape の ShapeData と同じ並び
cbuffer ShapeCb : register(b1){
	float4 shapeColor;	// 線の色
	float value;		// 使わない
	float time;			// 経過時間（秒）
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
	const float nearCorner = step(1.0f - CORNER_LENGTH, p.x) * step(1.0f - CORNER_LENGTH, p.y);
	const float onEdge = max(step(1.0f - LINE_WIDTH, p.x), step(1.0f - LINE_WIDTH, p.y));
	const float alpha = nearCorner * onEdge;
	if (alpha <= 0.0f)
	{
		discard;
	}
	const float blink = 0.8f + 0.2f * sin(time * BLINK_SPEED);
	return float4(shapeColor.rgb, shapeColor.a * alpha * blink);
}
