/*!
 * @brief	キャラの輪郭線（CharacterOutline）。頂点を法線の向きへふくらませ、裏の面だけを光る色で描く
 * ふくらませる量はカメラからの距離に比例させる（遠くの敵でも、画面の上では同じくらいの太さの線になる）
 */

cbuffer ModelCb : register(b0)
{
	float4x4 mWorld;
	float4x4 mView;
	float4x4 mProj;
};

// CharacterOutline の Constant と同じ並び
cbuffer OutlineCb : register(b1)
{
	float3 cameraPos;	// カメラの位置
	float width;		// 線の太さ（カメラからの距離に掛ける）
	float3 color;		// 線の色
	float pad;
};

struct VSInput
{
	float4 pos : POSITION;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD0;
	float3 tangent : TANGENT;
	float3 biNormal : BINORMAL;
	int4 indices : BLENDINDICES0;
	float4 weights : BLENDWEIGHT0;
};

struct PSInput
{
	float4 pos : SV_POSITION;
};

PSInput VSMain(VSInput In)
{
	PSInput Out;
	float4 worldPos = mul(mWorld, In.pos);
	// mWorld には大きさも入っているので、向きだけを使うために正規化する
	const float3 worldNormal = normalize(mul((float3x3)mWorld, In.normal));
	worldPos.xyz += worldNormal * width * distance(worldPos.xyz, cameraPos);
	Out.pos = mul(mProj, mul(mView, worldPos));
	return Out;
}

float4 PSMain(PSInput In) : SV_Target0
{
	return float4(color, 1.0f);
}
