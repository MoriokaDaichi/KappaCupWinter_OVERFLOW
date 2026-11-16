#include "stdafx.h"
#include "camera/ScreenPosition.h"

namespace nsScreen
{
	bool CalcScreenPosition(const Vector3& worldPos, Vector2& screenPos)
	{
		// カメラの前向きとの内積がマイナスなら、カメラの後ろにある
		const Vector3 toTarget = worldPos - g_camera3D->GetPosition();
		if (toTarget.Dot(g_camera3D->GetForward()) <= 0.0f)
		{
			return false;
		}

		g_camera3D->CalcScreenPositionFromWorldPosition(screenPos, worldPos);
		return true;
	}

	bool IsInCenterBox(const Vector2& screenPos, const Vector2& halfSize)
	{
		// 原点が画面の中央なので、絶対値が半分の大きさ以下なら中に入っている
		return fabsf(screenPos.x) <= halfSize.x && fabsf(screenPos.y) <= halfSize.y;
	}
}
