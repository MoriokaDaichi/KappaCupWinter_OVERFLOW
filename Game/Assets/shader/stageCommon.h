/*!
 * @brief	ステージの見た目のシェーダーで共通の色と計算
 */

static const float3 BACKGROUND_COLOR = float3(0.008f, 0.010f, 0.045f);	// 背景の紺
static const float3 HORIZON_COLOR = float3(0.10f, 0.02f, 0.16f);		// 地平線のあたりの光
static const float3 CYAN = float3(0.0f, 0.90f, 1.0f);					// #00e5ff
static const float3 MAGENTA = float3(1.0f, 0.17f, 0.84f);				// #ff2bd6

static const float HORIZON_SHARPNESS = 6.0f;	// 地平線の光の細さ
static const float FOG_DISTANCE = 12000.0f;		// この距離でほぼ背景の色になる
static const float PULSE_SPEED = 2400.0f;		// 流れる光の速さ（+Z へ流れる）
static const float PULSE_INTERVAL = 4000.0f;	// 流れる光の間隔
static const float PULSE_SHARPNESS = 24.0f;		// 流れる光の尾の短さ

/** 見ている向きごとの空の色（線や星を除く）。遠くの物は、この色に溶かす */
float3 CalcSkyColor(const float3 direction)
{
	const float up = abs(direction.y) / max(length(direction), 0.001f);
	return BACKGROUND_COLOR + HORIZON_COLOR * exp(-up * HORIZON_SHARPNESS);
}

/** 進む向きへ流れる光の強さ（0〜1）を返す。先頭が 1 で、後ろへ尾を引く */
float CalcPulse(const float z, const float time)
{
	return pow(frac((z - time * PULSE_SPEED) / PULSE_INTERVAL), PULSE_SHARPNESS);
}

/** 遠いほど空の色に溶かす。溶けきった所は描かない（空の色の板が、奥の遠景のビルを隠さないように） */
float3 ApplyFog(const float3 color, const float3 toPixel)
{
	const float fogRate = dot(toPixel, toPixel) / (FOG_DISTANCE * FOG_DISTANCE);
	clip(1.0f - fogRate);
	const float fog = saturate(fogRate);
	return lerp(color, CalcSkyColor(toPixel), fog);
}
