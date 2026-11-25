/*!
 * @brief	落ちたときの「SIGNAL LOST」の画面（企画書の失敗の絵）。画面を暗くし、赤い横の乱れと走査線を出す
 * value: 出てからの割合（0〜1）。出た瞬間に強く乱れ、だんだん暗くなる
 */

static const float3 NOISE_RED = float3(1.0f, 0.15f, 0.2f);	// 乱れの赤
static const float BAND_COUNT = 40.0f;						// 横の帯の数
static const float SCANLINE_COUNT = 360.0f;					// 走査線の本数

cbuffer cb : register(b0){
	float4x4 mvp;		// モデルビュープロジェクション行列
	float4 mulColor;	// 乗算カラー（使わない）
};

// HudShape の ShapeData と同じ並び
cbuffer ShapeCb : register(b1){
	float4 shapeColor;	// 使わない
	float value;		// 出てからの割合（0〜1）
	float time;			// 経過時間（秒）。乱れを動かす
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

/** 0〜1 の疑似乱数 */
float Hash(float n)
{
	return frac(sin(n * 12.9898f) * 43758.5453f);
}

float4 PSMain(PSInput In) : SV_Target0
{
	const float2 uv = In.uv;

	// だんだん暗くする（最後はほとんど黒。戻るときの瞬間移動を隠す）
	const float dark = 0.35f + 0.6f * value;

	// 横の帯ごとに、ときどき赤い乱れを出す。出た瞬間ほど多い
	const float band = floor(uv.y * BAND_COUNT);
	const float tick = floor(time * 20.0f);
	const float noise = Hash(band * 7.13f + tick * 1.37f);
	const float threshold = 0.75f + 0.2f * value;
	const float streakLength = Hash(band + tick * 3.1f);
	const float streak = step(threshold, noise) * step(abs(uv.x - Hash(band * 3.7f + tick)), streakLength * 0.4f);

	const float scan = 0.5f + 0.5f * sin(uv.y * SCANLINE_COUNT * 3.14159f);

	float3 color = NOISE_RED * streak * 0.8f;
	float alpha = max(dark, streak * 0.8f) * (0.85f + 0.15f * scan);
	return float4(color, saturate(alpha));
}
