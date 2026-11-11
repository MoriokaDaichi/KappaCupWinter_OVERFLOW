#include "stdafx.h"
#include "weapon/WarningLine.h"

#include "collision/RayCast.h"

void WarningLine::Init()
{
	// 暗い場所でも見落とさないように、ライトに関係なく光らせる
	ModelInitData initData;
	initData.m_tkmFilePath = "Assets/modelData/debug/debug_wire.tkm";
	initData.m_fxFilePath = "Assets/shader/warningLine.fx";
	// 既定の Z-up だとエンジンが X 軸で -90 度回してから伸ばすので、長さの軸（Z）がずれる。回さない Y-up で読む（GrappleWire と同じ）
	initData.m_modelUpAxis = enModelUpAxisY;
	initData.m_expandConstantBuffer = &m_constant;
	initData.m_expandConstantBufferSize = sizeof(m_constant);
	// 加算で描き、奥の物を隠さないように深度は書かない（カメラの近くを薄くするため。warningLine.fx を参照）
	initData.m_alphaBlendMode = AlphaBlendMode_Add;
	initData.m_isDepthWrite = false;
	m_modelRender.InitForwardRendering(initData);
	// 細い線の影は目立って不自然なので落とさない
	m_modelRender.SetShadowCasterFlag(false);
}

void WarningLine::Update(const bool isVisible, const Vector3& start, const Vector3& direction, const float length)
{
	m_canDraw = false;
	if (!isVisible)
	{
		m_visibleTime = 0.0f;
		return;
	}

	m_visibleTime += g_gameTime->GetFrameDeltaTime();

	Vector3 lineDirection = direction;
	lineDirection.Normalize();

	// 壁があれば壁で止める（弾も壁で止まるので、線が弾の行き先をそのまま示す）
	float drawLength = length;
	const RayHitInfo hit = nsCollision::RayCast(start, start + lineDirection * length);
	if (hit.isHit)
	{
		drawLength = (hit.position - start).Length();
	}

	if (drawLength < MIN_LENGTH)
	{
		return;
	}

	// 出始めは細く、growTime かけて太くする（急に太い線が出ると、狙われ始めたのか撃たれたのか分かりにくいため）
	const float growRate = m_param.growTime > 0.0f ? min(m_visibleTime / m_param.growTime, 1.0f) : 1.0f;
	const float thickness = m_param.startThickness + (m_param.thickness - m_param.startThickness) * growRate;
	const float thicknessScale = thickness / MODEL_DIAMETER;

	// モデルは Z 方向に伸びているので、Z 軸を線の向きへ回す
	Quaternion rotation;
	rotation.SetRotation(Vector3::AxisZ, lineDirection);

	// 原点がモデルの端にあるので、位置は始まりのままでいい
	m_modelRender.SetTRS(start, rotation, Vector3(thicknessScale, thicknessScale, drawLength / MODEL_LENGTH));
	m_modelRender.Update();

	m_constant.cameraPos = g_camera3D->GetPosition();
	m_constant.brightness = m_param.brightness;
	m_constant.time = m_visibleTime;
	m_canDraw = true;
}

void WarningLine::Draw(RenderContext& rc)
{
	if (!m_canDraw)
	{
		return;
	}

	m_modelRender.Draw(rc);
}
