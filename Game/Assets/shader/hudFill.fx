/*!
 * @brief	板を 1 色で塗る（うすい走査線つき）。ポーズの間、ゲームの画面を暗くするのに使う
 */

static const float SCANLINE_COUNT = 300.0f;	// 走査線の本数
static const float SCANLINE_DEPTH = 0.25f;	// 走査線で薄くなる割合

cbuffer cb : register(b0){
	float4x4 mvp;		// モデルビュープロジェクション行列
	float4 mulColor;	// 乗算カラー（使わない）
};

// HudShape の ShapeData と同じ並び
cbuffer ShapeCb : register(b1){
	float4 shapeColor;	// 塗る色（a で濃さ）
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
	const float scan = 1.0f - SCANLINE_DEPTH * (0.5f + 0.5f * sin(In.uv.y * SCANLINE_COUNT * 3.14159f));
	return float4(shapeColor.rgb, shapeColor.a * scan);
}
