/*!
 *@brief	レンダリング設定(ライト、ポストエフェクト)の保存と読み込み。
 */

#include "k2EnginePreCompile.h"
#include "RenderSettings.h"

namespace nsK2Engine {
	namespace nsDbg {

		namespace {
			/// <summary>
			/// ディレクショナルライトのキー名を作る。
			/// </summary>
			void MakeDirectionalLightKey(
				char* buffer, int bufferSize, int lightNo, const char* name)
			{
				sprintf_s(buffer, bufferSize, "light.directional.%d.%s", lightNo, name);
			}
		}

		const char* GetRenderSettingsFilePath()
		{
			return "Assets/settings/renderSettings.txt";
		}

		bool SaveRenderSettings()
		{
			if (g_renderingEngine == nullptr) {
				return false;
			}
			DebugSettings settings;

			// ライト。
			for (int lightNo = 0; lightNo < MAX_DIRECTIONAL_LIGHT; lightNo++) {
				const auto& light = g_renderingEngine->GetDirectionLight(lightNo);
				char key[128];
				MakeDirectionalLightKey(key, sizeof(key), lightNo, "direction");
				settings.SetVector3(key, light.direction);
				MakeDirectionalLightKey(key, sizeof(key), lightNo, "color");
				settings.SetVector3(key, { light.color.x, light.color.y, light.color.z });
				MakeDirectionalLightKey(key, sizeof(key), lightNo, "castShadow");
				settings.SetBool(key, light.castShadow != 0);
			}
			settings.SetVector3("light.ambient", g_renderingEngine->GetAmbient());

			// ポストエフェクト。
			settings.SetBool("postEffect.bloom.enable", g_renderingEngine->IsEnableBloom());
			settings.SetFloat("postEffect.bloom.threshold", g_renderingEngine->GetBloomThreshold());
			settings.SetBool("postEffect.tonemap.enable", g_renderingEngine->IsEnableTonemap());
			settings.SetFloat("postEffect.tonemap.middleGray", g_renderingEngine->GetSceneMiddleGray());
			settings.SetBool("postEffect.fxaa.enable", g_renderingEngine->IsEnableFxaa());
			settings.SetBool("postEffect.dof.enable", g_renderingEngine->IsEnableDof());
			settings.SetBool("postEffect.ssr.enable", g_renderingEngine->IsEnableSsr());

			return settings.Save(GetRenderSettingsFilePath());
		}

		bool LoadRenderSettings()
		{
			if (g_renderingEngine == nullptr) {
				return false;
			}
			DebugSettings settings;
			if (settings.Load(GetRenderSettingsFilePath()) == false) {
				return false;
			}

			// ライト。
			for (int lightNo = 0; lightNo < MAX_DIRECTIONAL_LIGHT; lightNo++) {
				const auto& light = g_renderingEngine->GetDirectionLight(lightNo);
				Vector3 direction = light.direction;
				Vector3 color = { light.color.x, light.color.y, light.color.z };
				bool castShadow = light.castShadow != 0;

				char key[128];
				MakeDirectionalLightKey(key, sizeof(key), lightNo, "direction");
				direction = settings.GetVector3(key, direction);
				MakeDirectionalLightKey(key, sizeof(key), lightNo, "color");
				color = settings.GetVector3(key, color);
				MakeDirectionalLightKey(key, sizeof(key), lightNo, "castShadow");
				castShadow = settings.GetBool(key, castShadow);

				g_renderingEngine->SetDirectionLight(lightNo, direction, color);
				g_renderingEngine->SetDirectionLightCastShadow(lightNo, castShadow);
			}
			g_renderingEngine->SetAmbient(
				settings.GetVector3("light.ambient", g_renderingEngine->GetAmbient()));

			// ポストエフェクト。
			g_renderingEngine->SetEnableBloom(
				settings.GetBool("postEffect.bloom.enable", g_renderingEngine->IsEnableBloom()));
			g_renderingEngine->SetBloomThreshold(
				settings.GetFloat("postEffect.bloom.threshold", g_renderingEngine->GetBloomThreshold()));

			if (settings.GetBool("postEffect.tonemap.enable", g_renderingEngine->IsEnableTonemap())) {
				g_renderingEngine->EnableTonemap();
			}
			else {
				g_renderingEngine->DisableTonemap();
			}
			g_renderingEngine->SetSceneMiddleGray(
				settings.GetFloat("postEffect.tonemap.middleGray", g_renderingEngine->GetSceneMiddleGray()));

			g_renderingEngine->SetEnableFxaa(
				settings.GetBool("postEffect.fxaa.enable", g_renderingEngine->IsEnableFxaa()));
			g_renderingEngine->SetEnableDof(
				settings.GetBool("postEffect.dof.enable", g_renderingEngine->IsEnableDof()));
			g_renderingEngine->SetEnableSsr(
				settings.GetBool("postEffect.ssr.enable", g_renderingEngine->IsEnableSsr()));

			return true;
		}
	}
}
