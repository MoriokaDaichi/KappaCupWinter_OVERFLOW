#include "stdafx.h"
#include "grapple/GrappleWire.h"

bool GrappleWire::Start()
{
	//既定の Z-up だとエンジンが X 軸で -90 度回してから伸ばすので、長さの軸（Z）がずれる。回さない Y-up で読む
	m_modelRender.Init("Assets/modelData/debug/debug_wire.tkm", nullptr, 0, enModelUpAxisY);
	//細いワイヤーの影は目立って不自然なので落とさない
	m_modelRender.SetShadowCasterFlag(false);

	return true;
}

void GrappleWire::Update()
{
	if (!m_isVisible)
	{
		return;
	}

	UpdateTransform();
	m_modelRender.Update();
}

void GrappleWire::Render(RenderContext& rc)
{
	if (!m_isVisible || !m_canDraw)
	{
		return;
	}

	m_modelRender.Draw(rc);
}

void GrappleWire::SetTarget(const Vector3& anchorPosition, const float tipLength)
{
	m_targetPosition = anchorPosition;
	m_tipLength = tipLength;
}

void GrappleWire::SetHandPosition(const Vector3& hand)
{
	m_start = hand;
}

void GrappleWire::UpdateTransform()
{
	Vector3 toAnchor = m_targetPosition - m_start;
	const float length = min(m_tipLength, toAnchor.Length());

	m_canDraw = length >= MIN_LENGTH;
	if (!m_canDraw)
	{
		return;
	}

	toAnchor.Normalize();

	//モデルは Z 方向に伸びているので、Z 軸をワイヤーの向きへ回す
	Quaternion rotation;
	rotation.SetRotation(Vector3::AxisZ, toAnchor);

	const Vector3 scale(m_param.thicknessScale, m_param.thicknessScale, length / MODEL_LENGTH);

	//原点がモデルの端にあるので、位置は始点のままでいい
	m_modelRender.SetTRS(m_start, rotation, scale);
}
