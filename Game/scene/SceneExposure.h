#pragma once

/**
 * エンジンの自動露出の目標の明るさ（SceneMiddleGray）を、最初の値に対する倍率で決める
 * 「今の値 × 倍率」にすると、シーンを作り直すたび（タイトル → ゲーム → タイトル → ゲーム）に暗くなっていくので、
 * 最初に読んだ値を覚えておき、いつもそれに掛ける
 */
namespace nsSceneExposure
{
	inline void SetScale(const float scale)
	{
		static const float s_baseMiddleGray = g_renderingEngine->GetSceneMiddleGray();
		g_renderingEngine->SetSceneMiddleGray(s_baseMiddleGray * scale);
	}
}
