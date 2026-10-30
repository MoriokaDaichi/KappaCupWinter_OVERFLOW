/*!
 * @brief	ロックオンのドーナツ（円の輪を、上から時計回りに progress の分だけ塗る）
 */

static const float PI2 = 6.28318530f;
static const float BACK_ALPHA = 0.25f;		// まだ満ちていないところの濃さ（輪の形が分かるように、うっすら出す）

cbuffer cb : register(b0){
	float4x4 mvp;		// モデルビュープロジェクション行列
	float4 mulColor;	// 乗算カラー（使わない）
};

// LockOnRing の RingData と同じ並び
cbuffer RingCb : register(b1){
	float4 ringColor;	// 輪の色
	float progress;		// 塗る割合（0〜1）
	float thickness;	// 輪の太さ（半径に対する割合）
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
	// uv（左上 0,0 〜 右下 1,1）を、中心 0,0・上が +y の -1〜1 に直す
	float2 p = In.uv * 2.0f - 1.0f;
	p.y = -p.y;

	// 輪の外と内側の穴は描かない。縁は 1 ピクセル分ぼかしてギザギザを消す
	float r = length(p);
	float edge = fwidth(r);
	float alpha = smoothstep(1.0f, 1.0f - edge, r) * smoothstep(1.0f - thickness - edge, 1.0f - thickness, r);
	if (alpha <= 0.0f) {
		discard;
	}

	// 真上を 0、時計回りに 1 まで増える割合
	float angleRate = atan2(p.x, p.y) / PI2;
	if (angleRate < 0.0f) {
		angleRate += 1.0f;
	}

	float4 color = ringColor;
	if (angleRate > progress) {
		color.a *= BACK_ALPHA;
	}
	color.a *= alpha;
	return color;
}
