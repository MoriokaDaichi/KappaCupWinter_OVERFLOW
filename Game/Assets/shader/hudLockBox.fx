/*!
 * @brief	ロックする範囲の四角（四隅の L 字）。板はいちばん大きい Lv の四角の大きさで作り、今の Lv の大きさを割合でもらう
 */

static const float LINE_WIDTH = 0.008f;		// 線の太さ（板の半分に対する割合）
static const float CORNER_LENGTH = 0.2f;	// L 字の長さ（今の四角の半分に対する割合）

cbuffer cb : register(b0){
	float4x4 mvp;		// モデルビュープロジェクション行列
	float4 mulColor;	// 乗算カラー（使わない）
};

// HudShape の ShapeData と同じ並び
cbuffer ShapeCb : register(b1){
	float4 shapeColor;	// 線の色
	float value;		// 今の四角の幅の、板の幅に対する割合（0〜1）
	float time;			// 使わない
	float flag;			// 使わない
	float value2;		// 今の四角の高さの、板の高さに対する割合（0〜1）
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
	// 板の中央から端までを 0〜1 にする（上下左右で同じ形になる）
	const float2 p = abs(In.uv * 2.0f - 1.0f);
	const float2 box = float2(value, value2);

	const float inBox = step(p.x, box.x) * step(p.y, box.y);
	const float nearCorner = step(box.x * (1.0f - CORNER_LENGTH), p.x) * step(box.y * (1.0f - CORNER_LENGTH), p.y);
	const float onEdge = max(step(box.x - LINE_WIDTH, p.x), step(box.y - LINE_WIDTH, p.y));
	const float alpha = inBox * nearCorner * onEdge;
	if (alpha <= 0.0f)
	{
		discard;
	}
	return float4(shapeColor.rgb, shapeColor.a * alpha);
}
