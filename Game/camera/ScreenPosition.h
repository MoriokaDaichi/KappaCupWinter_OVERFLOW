#pragma once

namespace nsScreen
{
	/**
	 * worldPos が画面のどこに映るかを screenPos に入れる（画面の中央が 0,0、右と上がプラス、単位はピクセル）
	 * カメラの後ろにあるときは false を返す。後ろの点もそのまま計算すると、上下左右が逆さになって画面の中に出てしまうため
	 */
	bool CalcScreenPosition(const Vector3& worldPos, Vector2& screenPos);
	/** screenPos が、画面の中央を中心にした四角（半分の幅・高さが halfSize）の中にあるか */
	bool IsInCenterBox(const Vector2& screenPos, const Vector2& halfSize);
}
