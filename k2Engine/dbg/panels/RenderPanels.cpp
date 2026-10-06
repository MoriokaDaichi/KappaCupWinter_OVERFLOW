/*!
 *@brief	ライトとポストエフェクトのデバッグパネル。
 */

#include "k2EnginePreCompile.h"
#include "RenderPanels.h"
#include "dbg/RenderSettings.h"

namespace nsK2Engine {
	namespace nsDbg {

#ifdef K2_ENABLE_DEBUG_GUI

		namespace {
			// 直前にSave/Loadした結果のメッセージ。
			std::string	g_settingsMessage;

			/// <summary>
			/// 設定の保存/読み込みボタンを描画する。
			/// </summary>
			void DrawSaveLoadButtons()
			{
				ImGui::Separator();
				if (ImGui::Button("Save")) {
					g_settingsMessage = SaveRenderSettings()
						? std::string("saved : ") + GetRenderSettingsFilePath()
						: std::string("save failed.");
				}
				ImGui::SetItemTooltip("%s", U8(
					"今の設定を Assets/settings/renderSettings.txt に保存する。\n"
					"メモ帳で開いて直接編集することもできる。"));
				ImGui::SameLine();
				if (ImGui::Button("Load")) {
					g_settingsMessage = LoadRenderSettings()
						? std::string("loaded : ") + GetRenderSettingsFilePath()
						: std::string("load failed. (file not found?)");
				}
				if (g_settingsMessage.empty() == false) {
					ImGui::TextDisabled("%s", g_settingsMessage.c_str());
				}
			}

			/// <summary>
			/// ライトのパネル。
			/// </summary>
			void DrawLightPanel()
			{
				if (g_renderingEngine == nullptr) {
					ImGui::TextUnformatted("RenderingEngine is not created.");
					return;
				}

				// 環境光。
				Vector3 ambient = g_renderingEngine->GetAmbient();
				if (ImGui::ColorEdit3(
						"ambient",
						&ambient.x,
						ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR)
				) {
					g_renderingEngine->SetAmbient(ambient);
				}
				ImGui::SetItemTooltip("%s", U8(
					"環境光。どこからともなく当たっている光。\n"
					"影の中の明るさがこれで決まる。"));

				ImGui::Separator();

				// ディレクショナルライト。
				for (int lightNo = 0; lightNo < MAX_DIRECTIONAL_LIGHT; lightNo++) {
					ImGui::PushID(lightNo);

					char header[64];
					sprintf_s(header, "Directional Light %d", lightNo);
					const bool isDefaultOpen = (lightNo == 0);
					if (ImGui::CollapsingHeader(
							header,
							isDefaultOpen ? ImGuiTreeNodeFlags_DefaultOpen : 0)
					) {
						const auto& light = g_renderingEngine->GetDirectionLight(lightNo);
						Vector3 direction = light.direction;
						Vector3 color = { light.color.x, light.color.y, light.color.z };
						bool castShadow = light.castShadow != 0;
						bool isChanged = false;

						if (ImGui::DragFloat3("direction", &direction.x, 0.01f, -1.0f, 1.0f)) {
							isChanged = true;
						}
						ImGui::SetItemTooltip("%s", U8(
							"光が進んでいく向き。\n"
							"{0, -1, 0} なら真上から真下に向かって光が当たる。\n"
							"長さは自動的に1に直される。"));

						if (ImGui::ColorEdit3(
								"color",
								&color.x,
								ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR)
						) {
							isChanged = true;
						}
						ImGui::SetItemTooltip("%s", U8(
							"ライトの色と強さ。\n"
							"1.0より大きい値を入れると、より強く光る。"));

						if (ImGui::Checkbox("cast shadow", &castShadow)) {
							g_renderingEngine->SetDirectionLightCastShadow(lightNo, castShadow);
						}
						ImGui::SetItemTooltip("%s", U8("このライトで影を落とすかどうか。"));

						if (isChanged) {
							// 向きの長さを1に直してから設定する。
							// 長さが0だと向きが決まらないので、その場合は何もしない。
							float length = direction.Length();
							if (length > 0.0001f) {
								direction.Scale(1.0f / length);
								g_renderingEngine->SetDirectionLight(lightNo, direction, color);
							}
						}
					}
					ImGui::PopID();
				}

				DrawSaveLoadButtons();
			}

			/// <summary>
			/// ポストエフェクトのパネル。
			/// </summary>
			void DrawPostEffectPanel()
			{
				if (g_renderingEngine == nullptr) {
					ImGui::TextUnformatted("RenderingEngine is not created.");
					return;
				}

				// ブルーム。
				bool isEnableBloom = g_renderingEngine->IsEnableBloom();
				if (ImGui::Checkbox("Bloom", &isEnableBloom)) {
					g_renderingEngine->SetEnableBloom(isEnableBloom);
				}
				ImGui::SetItemTooltip("%s", U8(
					"明るい部分をにじませる効果。\n"
					"光っているものをより光って見せることができる。"));
				if (isEnableBloom) {
					ImGui::Indent();
					float threshold = g_renderingEngine->GetBloomThreshold();
					if (ImGui::DragFloat("threshold", &threshold, 0.01f, 0.0f, 10.0f)) {
						g_renderingEngine->SetBloomThreshold(threshold);
					}
					ImGui::SetItemTooltip("%s", U8(
						"この明るさを超えた部分だけがにじむ。\n"
						"小さくするほど画面全体がぼんやり光る。"));
					ImGui::Unindent();
				}

				ImGui::Separator();

				// トーンマップ。
				bool isEnableTonemap = g_renderingEngine->IsEnableTonemap();
				if (ImGui::Checkbox("Tone map", &isEnableTonemap)) {
					isEnableTonemap
						? g_renderingEngine->EnableTonemap()
						: g_renderingEngine->DisableTonemap();
				}
				ImGui::SetItemTooltip("%s", U8(
					"明るさを画面に表示できる範囲に収める処理。\n"
					"切ると、明るいところが真っ白につぶれる。"));
				if (isEnableTonemap) {
					ImGui::Indent();
					float middleGray = g_renderingEngine->GetSceneMiddleGray();
					if (ImGui::DragFloat("middle gray", &middleGray, 0.005f, 0.0f, 2.0f)) {
						g_renderingEngine->SetSceneMiddleGray(middleGray);
					}
					ImGui::SetItemTooltip("%s", U8(
						"シーン全体の明るさの基準。慣習的に0.18が使われる。\n"
						"カメラの露出のようなもの。"));
					ImGui::Unindent();
				}

				ImGui::Separator();

				// FXAA。
				bool isEnableFxaa = g_renderingEngine->IsEnableFxaa();
				if (ImGui::Checkbox("FXAA", &isEnableFxaa)) {
					g_renderingEngine->SetEnableFxaa(isEnableFxaa);
				}
				ImGui::SetItemTooltip("%s", U8(
					"輪郭のギザギザをなめらかにする処理(アンチエイリアス)。\n"
					"切って輪郭を拡大して見比べてみよう。"));

				// 被写界深度。
				bool isEnableDof = g_renderingEngine->IsEnableDof();
				if (ImGui::Checkbox("Depth of field", &isEnableDof)) {
					g_renderingEngine->SetEnableDof(isEnableDof);
				}
				ImGui::SetItemTooltip("%s", U8(
					"ピントの合っていない距離をぼかす効果。\n"
					"カメラで写真を撮ったときのボケ。"));

				// SSR。
				bool isEnableSsr = g_renderingEngine->IsEnableSsr();
				if (ImGui::Checkbox("SSR (screen space reflection)", &isEnableSsr)) {
					g_renderingEngine->SetEnableSsr(isEnableSsr);
				}
				ImGui::SetItemTooltip("%s", U8(
					"床などへの映り込みを描く処理。\n"
					"レイトレーシングが有効なときは、こちらは使われない。"));

				if (g_renderingEngine->IsEnableRaytracing()) {
					ImGui::TextDisabled("%s", U8("(レイトレーシングが有効なので、SSRは実行されません)"));
				}

				DrawSaveLoadButtons();
			}
		}

		void RegisterRenderPanels()
		{
			DebugGui::RegisterPanel("Graphics", "Light", DrawLightPanel);
			DebugGui::RegisterPanel("Graphics", "Post Effect", DrawPostEffectPanel);
		}

#else // K2_ENABLE_DEBUG_GUI

		void RegisterRenderPanels() {}

#endif // K2_ENABLE_DEBUG_GUI
	}
}
