/*!
 * @brief	ステージの箱・遠景のビル・穴の底と、背景（暗い面に、ふちとグリッドがネオンで光る）
 * 0 番のインスタンスは背景。カメラの位置に置き、いちばん奥の深度で描く
 */

#include "stageCommon.h"

static const float MODEL_HALF_SIZE = 50.0f;		// 箱のモデル（debug_wall.tkm）の 1 辺の半分
static const float BACKGROUND_DEPTH = 0.9999999f;	// 背景の深度（遠景のビルより必ず奥）

// StageRenderer::Kind と同じ番号
static const uint KIND_BLOCK = 0;
static const uint KIND_TOWER = 1;
static const uint KIND_HAZARD = 2;
static const uint KIND_PANEL = 3;

static const float3 FLOOR_COLOR = float3(0.012f, 0.018f, 0.070f);		// 床の面
static const float3 WALL_COLOR = float3(0.020f, 0.008f, 0.060f);		// 壁の面（少し紫）
static const float3 HAZARD_COLOR = float3(1.0f, 0.12f, 0.03f);			// 穴の底の色
static const float3 TOWER_COLOR = float3(0.004f, 0.005f, 0.022f);		// 遠景のビルの面

static const float EDGE_BRIGHTNESS = 12.0f;		// ふちの明るさ（1 より大きいと Bloom で光る）
static const float EDGE_WIDTH = 8.0f;			// ふちの線の太さ
static const float GLOW_WIDTH = 45.0f;			// ふちのまわりのにじみの幅
static const float GLOW_BRIGHTNESS = 0.5f;		// にじみの明るさ
static const float GRID_BRIGHTNESS = 1.5f;		// グリッドの明るさ
static const float GRID_WIDTH = 5.0f;			// グリッドの線の太さ
static const float FLOOR_GRID = 500.0f;			// 床のグリッドの間隔
static const float WALL_GRID = 1000.0f;			// 壁のグリッドの間隔
static const float PULSE_BRIGHTNESS = 5.0f;		// 流れる光の明るさ（線の明るさに掛ける）

static const float MASK_TILE = 2000.0f;			// 模様 1 枚が覆う長さ
static const float MASK_BRIGHTNESS = 0.14f;		// 壁の模様の明るさ（敵やアンカーより目立たないように、暗くする）

static const float TOWER_TILE = 1500.0f;		// ビルの模様 1 枚が覆う長さ
static const float TOWER_BRIGHTNESS = 0.3f;		// ビルの模様の明るさ（アンカーより目立たないように、暗くする）
static const float TOWER_EDGE_BRIGHTNESS = 0.8f;// ビルのふちの明るさ
static const float TOWER_EDGE_WIDTH = 60.0f;	// ビルのふちの太さ
static const float TOWER_FOG_DISTANCE = 40000.0f;	// ビルが空に溶けきる距離

static const float3 PANEL_COLOR = float3(0.020f, 0.008f, 0.060f);		// 壁走りの板の面（壁と同じ）
static const float PANEL_EDGE_BRIGHTNESS = 6.0f;	// 板のふちの明るさ
static const float PANEL_LINE_OFFSET = 300.0f;		// 横に走る光のラインの、中心からの高さ
static const float PANEL_LINE_WIDTH = 10.0f;		// 光のラインの太さ
static const float PANEL_LINE_BRIGHTNESS = 1.5f;	// 光のラインの明るさ

static const float HAZARD_STRIPE = 300.0f;		// 穴の底のしま模様の間隔
static const float HAZARD_SPEED = 300.0f;		// しま模様の流れる速さ
static const float HAZARD_BLINK = 3.0f;			// 明滅の速さ
static const float HAZARD_BRIGHTNESS = 3.0f;	// 穴の底の明るさ

static const float GROUND_HEIGHT = -6000.0f;	// 背景の地面の高さ
static const float GROUND_GRID = 3000.0f;		// 地面のグリッドの間隔
static const float GROUND_GRID_WIDTH = 30.0f;	// 地面のグリッドの太さ
static const float GROUND_FADE = 60000.0f;		// この距離で地面が見えなくなる
static const float GROUND_BRIGHTNESS = 1.6f;	// 地面のグリッドの明るさ
static const float STAR_SCALE = 120.0f;			// 空の点の細かさ（大きいほど小さく多い）
static const float STAR_RATE = 0.012f;			// 点が光るマスの割合
static const float STAR_BRIGHTNESS = 1.4f;		// 点の明るさ

cbuffer ModelCb : register(b0)
{
	float4x4 mWorld;
	float4x4 mView;
	float4x4 mProj;
};

// StageRenderer の Constant と同じ並び
cbuffer StageCb : register(b1)
{
	float g_time;	// 経過時間（秒）
};

// StageRenderer の Instance と同じ並び
struct Instance
{
	float4 center;		// 中心の位置
	float4 halfSize;	// xyz が大きさの半分。w が種類（Kind）
};

StructuredBuffer<Instance> g_instances : register(t10);
Texture2D<float4> g_wallMask : register(t11);	// 壁の模様（R=回路 G=パネル B=データ）
SamplerState g_sampler : register(s0);

struct VSInput
{
	float4 pos : POSITION;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD0;
	float3 tangent : TANGENT;
	float3 biNormal : BINORMAL;
	int4 indices : BLENDINDICES0;
	float4 weights : BLENDWEIGHT0;
	uint instanceId : SV_InstanceID;
};

struct PSInput
{
	float4 pos : SV_POSITION;
	float3 worldPos : TEXCOORD0;
	nointerpolation float3 center : TEXCOORD1;
	nointerpolation float3 halfSize : TEXCOORD2;
	nointerpolation uint isBackground : TEXCOORD3;
	nointerpolation uint kind : TEXCOORD4;
};

PSInput VSMain(VSInput In)
{
	const Instance instance = g_instances[In.instanceId];

	PSInput Out;
	Out.center = instance.center.xyz;
	Out.halfSize = instance.halfSize.xyz;
	Out.isBackground = (In.instanceId == 0) ? 1 : 0;
	Out.kind = (uint)instance.halfSize.w;
	// 立方体はどの軸も同じ形なので、モデルの回転は気にせず、大きさだけを掛ける
	Out.worldPos = instance.center.xyz + In.pos.xyz / MODEL_HALF_SIZE * instance.halfSize.xyz;
	Out.pos = mul(mProj, mul(mView, float4(Out.worldPos, 1.0f)));

	if (Out.isBackground != 0)
	{
		Out.pos.z = Out.pos.w * BACKGROUND_DEPTH;
	}
	return Out;
}

/** 線の濃さを返す。lineDistance は線からの距離。遠くで線が細くなりすぎたら、ちらつかないように薄くする */
float CalcLine(const float lineDistance, const float width, const float pixelSize)
{
	const float density = 1.0f - smoothstep(width, width + pixelSize, lineDistance);
	return density * saturate(width * 2.0f / pixelSize);
}

/** グリッドの線の濃さを返す */
float CalcGrid(const float2 position, const float interval, const float lineWidth)
{
	const float2 pixelSize = max(fwidth(position), 0.001f);
	const float2 toLine = abs(frac(position / interval + 0.5f) - 0.5f) * interval;
	return max(CalcLine(toLine.x, lineWidth, pixelSize.x), CalcLine(toLine.y, lineWidth, pixelSize.y));
}

/** 2 次元の値から、0〜1 の決まった乱数を作る */
float Hash(const float2 value)
{
	return frac(sin(dot(value, float2(12.9898f, 78.233f))) * 43758.5453f);
}

/** 背景（空の点、地平線の光、遠くの地面のグリッド） */
float3 CalcBackground(const float3 direction, const float3 cameraPos)
{
	const float3 ray = normalize(direction);
	float3 color = CalcSkyColor(ray);

	if (ray.y < 0.0f)
	{
		const float2 ground = cameraPos.xz + ray.xz * (GROUND_HEIGHT - cameraPos.y) / ray.y;
		const float groundDistance = length(ground - cameraPos.xz);
		const float pulse = CalcPulse(ground.y, g_time);
		const float density = CalcGrid(ground, GROUND_GRID, GROUND_GRID_WIDTH) * saturate(1.0f - groundDistance / GROUND_FADE);
		color += lerp(MAGENTA, CYAN, pulse) * density * GROUND_BRIGHTNESS * (1.0f + pulse * PULSE_BRIGHTNESS);
	}
	else
	{
		const float2 cell = floor(float2(atan2(ray.x, ray.z), ray.y) * STAR_SCALE);
		const float star = step(1.0f - STAR_RATE, Hash(cell));
		color += lerp(CYAN, MAGENTA, Hash(cell + 1.0f)) * star * STAR_BRIGHTNESS;
	}
	return color;
}

/** 遠景のビル（暗い面に、データの模様とふち） */
float3 CalcTower(const float2 facePos, const float edgeDistance, const float edgePixelSize)
{
	const float mask = g_wallMask.Sample(g_sampler, facePos / TOWER_TILE).b;
	float3 color = TOWER_COLOR + CYAN * mask * TOWER_BRIGHTNESS;
	color += MAGENTA * CalcLine(edgeDistance, TOWER_EDGE_WIDTH, edgePixelSize) * TOWER_EDGE_BRIGHTNESS;
	return color;
}

/** 壁走りの板（壁と同じマゼンタのふちと、横に走る光のライン。矢印とシアンはダッシュ板のために使わない）。isSide は横を向いた面か */
float3 CalcPanel(const float3 worldPos, const float3 center, const bool isSide, const float edgeDistance, const float edgePixelSize)
{
	float3 color = PANEL_COLOR;
	if (isSide)
	{
		const float toLine = abs(abs(worldPos.y - center.y) - PANEL_LINE_OFFSET);
		const float pulse = CalcPulse(worldPos.z, g_time);
		color += MAGENTA * CalcLine(toLine, PANEL_LINE_WIDTH, max(fwidth(toLine), 0.001f)) * PANEL_LINE_BRIGHTNESS * (1.0f + pulse * PULSE_BRIGHTNESS);
	}
	return lerp(color, MAGENTA * PANEL_EDGE_BRIGHTNESS, CalcLine(edgeDistance, EDGE_WIDTH, edgePixelSize));
}

/** 穴の底（赤いしま模様が流れて、明滅する） */
float3 CalcHazard(const float3 worldPos)
{
	const float stripe = step(0.5f, frac((worldPos.x + worldPos.z + g_time * HAZARD_SPEED) / HAZARD_STRIPE));
	const float blink = 0.6f + 0.4f * sin(g_time * HAZARD_BLINK);
	return HAZARD_COLOR * (0.25f + 0.75f * stripe) * blink * HAZARD_BRIGHTNESS;
}

float4 PSMain(PSInput In) : SV_Target0
{
	const float3 toPixel = In.worldPos - g_instances[0].center.xyz;

	if (In.isBackground != 0)
	{
		return float4(CalcBackground(toPixel, g_instances[0].center.xyz), 1.0f);
	}

	if (In.kind == KIND_HAZARD)
	{
		return float4(ApplyFog(CalcHazard(In.worldPos), toPixel), 1.0f);
	}

	// 面の向きから、面の上の 2 つの軸を選ぶ
	const float3 normal = abs(normalize(cross(ddx(In.worldPos), ddy(In.worldPos))));
	const float3 local = In.worldPos - In.center;
	const float3 toEdge = In.halfSize - abs(local);

	float2 facePos;
	float edgeDistance;
	float3 baseColor;
	float3 lineColor;
	float gridInterval;
	float brightness = 1.0f;
	bool isWall = true;

	if (normal.y > normal.x && normal.y > normal.z)
	{
		facePos = In.worldPos.xz;
		edgeDistance = min(toEdge.x, toEdge.z);
		baseColor = FLOOR_COLOR;
		lineColor = CYAN;
		gridInterval = FLOOR_GRID;
		isWall = false;
	}
	else if (normal.x > normal.z)
	{
		facePos = In.worldPos.zy;
		edgeDistance = min(toEdge.z, toEdge.y);
		baseColor = WALL_COLOR;
		lineColor = MAGENTA;
		gridInterval = WALL_GRID;
	}
	else
	{
		// 正面を向いた面は少し暗くして、横の壁と見分けやすくする
		facePos = In.worldPos.xy;
		edgeDistance = min(toEdge.x, toEdge.y);
		baseColor = WALL_COLOR * 0.6f;
		lineColor = MAGENTA;
		gridInterval = WALL_GRID;
		brightness = 0.8f;
	}

	const float edgePixelSize = max(fwidth(edgeDistance), 0.001f);

	if (In.kind == KIND_TOWER)
	{
		const float towerFog = saturate(length(toPixel) / TOWER_FOG_DISTANCE);
		return float4(lerp(CalcTower(facePos, edgeDistance, edgePixelSize), CalcSkyColor(toPixel), towerFog), 1.0f);
	}

	if (In.kind == KIND_PANEL)
	{
		const bool isSide = normal.x > normal.y && normal.x > normal.z;
		return float4(ApplyFog(CalcPanel(In.worldPos, In.center, isSide, edgeDistance, edgePixelSize), toPixel), 1.0f);
	}

	const float pulse = CalcPulse(In.worldPos.z, g_time);
	const float lineScale = 1.0f + pulse * PULSE_BRIGHTNESS;

	float3 color = baseColor;
	if (isWall)
	{
		// 模様は壁のマスごとに 3 種類から選ぶ。継ぎ目はグリッドの線に隠れる
		const float2 maskCell = floor(facePos / MASK_TILE);
		const float channel = floor(Hash(maskCell) * 3.0f);
		const float4 maskColor = g_wallMask.Sample(g_sampler, facePos / MASK_TILE);
		const float mask = (channel < 1.0f) ? maskColor.r : ((channel < 2.0f) ? maskColor.g : maskColor.b);
		color += lineColor * mask * MASK_BRIGHTNESS * lineScale;
	}
	color += lineColor * CalcGrid(facePos, gridInterval, GRID_WIDTH) * GRID_BRIGHTNESS * lineScale;
	color += lineColor * exp(-edgeDistance / GLOW_WIDTH) * GLOW_BRIGHTNESS;
	color = lerp(color, lineColor * EDGE_BRIGHTNESS, CalcLine(edgeDistance, EDGE_WIDTH, edgePixelSize));
	color *= brightness;

	// 遠くほど背景に溶かして、奥行きを出す（背景はカメラの位置にある）
	return float4(ApplyFog(color, toPixel), 1.0f);
}
