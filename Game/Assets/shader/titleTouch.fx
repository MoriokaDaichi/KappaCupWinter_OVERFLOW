/*!
 * @brief	タイトルで、親指が画面に触れている所の輪（企画書の絵の、親指のまわりのシアンの輪）
 * ふだんはゆっくり脈打ち、押すと外へ広がって消える。value: 押し込んだ量（0〜1）
 */

static const float3 RING_COLOR = float3(0.0f, 0.9f, 1.0f);	// 輪のシアン
static const float RING_RADIUS = 0.30f;						// ふだんの輪の大きさ（板の半分に対する割合）
static const float RING_SPREAD = 0.65f;						// 押したときに広がる量
static const float RING_WIDTH = 0.025f;						// 輪の太さ

cbuffer cb : register(b0){
	float4x4 mvp;		// モデルビュープロジェクション行列
	float4 mulColor;	// 乗算カラー（使わない）
};

// HudShape の ShapeData と同じ並び
cbuffer ShapeCb : register(b1){
	float4 shapeColor;	// 使わない
	float value;		// 押し込んだ量（0〜1）
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
	const float r = length(In.uv * 2.0f - 1.0f);
	const float idle = RING_RADIUS + 0.03f * sin(time * 3.0f);
	const float radius = lerp(idle, idle + RING_SPREAD, value);
	const float ring = 1.0f - smoothstep(RING_WIDTH * 0.5f, RING_WIDTH, abs(r - radius));
	const float alpha = ring * lerp(0.75f, 0.0f, value);
	if (alpha <= 0.0f)
	{
		discard;
	}
	return float4(RING_COLOR, alpha);
}
