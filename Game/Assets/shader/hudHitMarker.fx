/*!
 * @brief	弾が敵に当たったときに照準のまわりへ出す、×の形の 4 本の短い線（ヒットマーク）
 * 出た瞬間がいちばん濃く、少し外へ広がりながら消える
 */

static const float GAP_START = 0.30f;	// 出た瞬間の、真ん中の空き（板の半分に対する割合）
static const float GAP_SPREAD = 0.18f;	// 消えるまでに、空きが広がる量
static const float LINE_LENGTH = 0.40f;	// 線の長さ
static const float LINE_WIDTH = 0.06f;	// 線の太さの半分
static const float INV_SQRT2 = 0.70710678f;

cbuffer cb : register(b0){
	float4x4 mvp;		// モデルビュープロジェクション行列
	float4 mulColor;	// 乗算カラー（使わない）
};

// HudShape の ShapeData と同じ並び
cbuffer ShapeCb : register(b1){
	float4 shapeColor;	// 線の色
	float value;		// 残りの割合（1 で出た瞬間、0 で消える）
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
	// 45 度回した座標で、照準の十字と同じように 4 本の線を描く（＝×の形）
	const float2 p = In.uv * 2.0f - 1.0f;
	const float2 q = abs(float2(p.x + p.y, p.x - p.y) * INV_SQRT2);

	const float gap = GAP_START + GAP_SPREAD * (1.0f - value);
	const float outer = gap + LINE_LENGTH;
	const float lineA = step(q.y, LINE_WIDTH) * step(gap, q.x) * step(q.x, outer);
	const float lineB = step(q.x, LINE_WIDTH) * step(gap, q.y) * step(q.y, outer);
	const float alpha = max(lineA, lineB) * value;
	if (alpha <= 0.0f)
	{
		discard;
	}
	return float4(shapeColor.rgb, shapeColor.a * alpha);
}
