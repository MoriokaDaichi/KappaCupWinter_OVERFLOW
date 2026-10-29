#include "stdafx.h"
#include "actor/HandBones.h"

void HandBones::Init(const ModelRender& model, const CharacterParam::Hands& hands)
{
	m_model = &model;
	m_leftHandBoneId = FindBone(hands.left);
	m_rightHandBoneId = FindBone(hands.right);
	m_rightMiddleBoneId = FindBone(hands.rightMiddle);
}

bool HandBones::GetRopeHandPosition(Vector3& outPosition) const
{
	if (m_leftHandBoneId < 0 || m_rightHandBoneId < 0)
	{
		return false;
	}

	outPosition = GetMiddle(m_leftHandBoneId, m_rightHandBoneId);
	return true;
}

bool HandBones::GetGrip(Matrix& outHandMatrix, Vector3& outGripPosition) const
{
	if (m_rightHandBoneId < 0 || m_rightMiddleBoneId < 0)
	{
		return false;
	}

	outHandMatrix = m_model->GetBone(m_rightHandBoneId)->GetWorldMatrix();
	outGripPosition = GetMiddle(m_rightHandBoneId, m_rightMiddleBoneId);
	return true;
}

int HandBones::FindBone(const std::string& name) const
{
	// エンジンは骨の名前を wchar_t で受け取る。骨の名前は英数字だけなので、1 文字ずつ広げればよい
	const std::wstring wideName(name.begin(), name.end());
	return m_model->FindBoneID(wideName.c_str());
}

Vector3 HandBones::GetMiddle(const int boneA, const int boneB) const
{
	const Matrix& a = m_model->GetBone(boneA)->GetWorldMatrix();
	const Matrix& b = m_model->GetBone(boneB)->GetWorldMatrix();
	return Vector3(
		(a.m[3][0] + b.m[3][0]) * 0.5f,
		(a.m[3][1] + b.m[3][1]) * 0.5f,
		(a.m[3][2] + b.m[3][2]) * 0.5f);
}
