/*!
 *@brief	処理負荷を表示するデバッグパネル。
 */

#include "k2EngineLowPreCompile.h"
#include "ProfilerPanel.h"

namespace nsK2EngineLow {
	namespace nsDbg {

#ifdef K2_ENABLE_DEBUG_GUI

		namespace {
			// 1フレームの目標時間(ミリ秒)。60fpsなら約16.6ms。
			const float TARGET_FRAME_TIME_MS = 1000.0f / 60.0f;

			/// <summary>
			/// 時間の長さに応じた色を返す。
			/// </summary>
			ImVec4 GetTimeColor(float timeMs)
			{
				if (timeMs > TARGET_FRAME_TIME_MS) {
					// 1フレームの予算を使い切っている。
					return ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
				}
				if (timeMs > TARGET_FRAME_TIME_MS * 0.5f) {
					// 予算の半分以上を使っている。
					return ImVec4(1.0f, 0.9f, 0.4f, 1.0f);
				}
				return ImVec4(0.4f, 1.0f, 0.5f, 1.0f);
			}

			/// <summary>
			/// フレーム時間とグラフを描画する。
			/// </summary>
			void DrawFrameTime()
			{
				const float frameTimeMs = DebugProfiler::GetFrameTimeMs();
				const float fps = (frameTimeMs > 0.0f) ? 1000.0f / frameTimeMs : 0.0f;

				ImGui::TextColored(GetTimeColor(frameTimeMs),
					"%.2f ms  (%.1f fps)", frameTimeMs, fps);
				ImGui::SameLine();
				ImGui::TextDisabled("  max %.2f ms", DebugProfiler::GetFrameTimeMsMax());
				ImGui::SetItemTooltip("%s", U8(
					"1フレームにかかった時間。\n"
					"60fpsで動かすには 16.6ms 以内に収める必要がある。\n"
					"FPS制限が有効なときは、余った時間は待ち時間になる。"));

				// フレーム時間のグラフ。
				// 縦軸の上限は、目標時間の2倍か、実際の最大値の大きいほう。
				const float graphMax =
					(std::max)(TARGET_FRAME_TIME_MS * 2.0f, DebugProfiler::GetFrameTimeMsMax());
				ImGui::PlotLines(
					"##frameTime",
					DebugProfiler::GetFrameTimeHistory(),
					DebugProfiler::GetFrameTimeHistoryNum(),
					0,
					nullptr,
					0.0f,
					graphMax,
					ImVec2(-1.0f, 60.0f)
				);
			}

			/// <summary>
			/// 処理時間の内訳を描画する。
			/// </summary>
			void DrawSections()
			{
				const int resultNum = DebugProfiler::GetSectionResultNum();
				if (resultNum == 0) {
					ImGui::TextDisabled("(measuring...)");
					return;
				}
				const ImGuiTableFlags flags =
					ImGuiTableFlags_Borders
					| ImGuiTableFlags_RowBg
					| ImGuiTableFlags_SizingStretchProp;
				if (ImGui::BeginTable("##sections", 2, flags) == false) {
					return;
				}
				ImGui::TableSetupColumn("Section");
				ImGui::TableSetupColumn("Time", ImGuiTableColumnFlags_WidthFixed, 80.0f);
				ImGui::TableHeadersRow();

				for (int i = 0; i < resultNum; i++) {
					const auto& result = DebugProfiler::GetSectionResult(i);
					ImGui::TableNextRow();

					ImGui::TableNextColumn();
					// 入れ子の深さの分だけ字下げする。
					if (result.depth > 0) {
						ImGui::Indent(result.depth * 16.0f);
					}
					ImGui::TextUnformatted(result.name != nullptr ? result.name : "?");
					if (result.depth > 0) {
						ImGui::Unindent(result.depth * 16.0f);
					}

					ImGui::TableNextColumn();
					// 待ち時間の区間は長くても問題ないので、警告色にしない。
					const bool isWaitSection =
						(result.name != nullptr && strstr(result.name, "Wait") != nullptr);
					const ImVec4 color = isWaitSection
						? ImVec4(0.6f, 0.6f, 0.6f, 1.0f)
						: GetTimeColor(result.timeMs);
					ImGui::TextColored(color, "%.3f ms", result.timeMs);
				}
				ImGui::EndTable();
				ImGui::TextDisabled("%s", U8("※ 0.25秒ごとの平均値"));
				ImGui::TextDisabled("%s", U8("※ EndFrame は待ち時間。長いほど余裕がある"));
				ImGui::SetItemTooltip("%s", U8(
					"EndFrame(Present/Wait) には、60fpsに合わせるための\n"
					"「何もしないで待っている時間」が含まれている。\n"
					"ここが短くなってきたら、処理が重くなってきたということ。"));
			}

			/// <summary>
			/// 描画の負荷を描画する。
			/// </summary>
			void DrawRenderStats()
			{
				ImGui::Text("Draw call : %d", DebugProfiler::GetDrawCallCount());
				ImGui::SetItemTooltip("%s", U8(
					"GPUに「描いて」と命令した回数。\n"
					"1回1回にCPUのコストがかかるので、\n"
					"同じモデルをたくさん出すときはインスタンシング描画を使う。"));

				ImGui::Text("Instance  : %d", DebugProfiler::GetDrawInstanceCount());
				ImGui::SetItemTooltip("%s", U8(
					"実際に描かれたモデルの数。\n"
					"インスタンシング描画を使うと、\n"
					"ドローコール1回でたくさんのインスタンスを描ける。"));
			}

			/// <summary>
			/// ゲームオブジェクトの数を描画する。
			/// </summary>
			void DrawGameObjectStats()
			{
				auto gameObjectManager = GameObjectManager::GetInstance();
				if (gameObjectManager == nullptr) {
					return;
				}
				ImGui::Text("GameObject : %d", gameObjectManager->GetGameObjectCount());
				ImGui::SetItemTooltip("%s", U8(
					"生きているゲームオブジェクトの数。\n"
					"ずっと増え続けている場合は、DeleteGOの消し忘れを疑うこと。\n"
					"内訳は GameObject List パネルで見られる。"));
			}

			/// <summary>
			/// メモリの使用量を描画する。
			/// </summary>
			void DrawMemoryStats()
			{
				ImGui::Text("Memory : %.1f MB", DebugProfiler::GetProcessMemoryMB());
				ImGui::SetItemTooltip("%s", U8(
					"このゲームが使っているメインメモリの量。\n"
					"遊んでいるうちにずっと増え続ける場合は、\n"
					"メモリリーク(解放し忘れ)を疑うこと。"));

				const float videoMemoryMB = DebugProfiler::GetVideoMemoryMB();
				if (videoMemoryMB < 0.0f) {
					ImGui::TextDisabled("VRAM   : N/A");
					return;
				}
				const float budgetMB = DebugProfiler::GetVideoMemoryBudgetMB();
				ImGui::Text("VRAM   : %.1f MB / %.1f MB", videoMemoryMB, budgetMB);
				ImGui::SetItemTooltip("%s", U8(
					"このゲームが使っているビデオメモリ(GPU側のメモリ)の量。\n"
					"テクスチャやモデルはここに置かれる。\n"
					"上限を超えると、動作が急激に重くなる。"));
				if (budgetMB > 0.0f) {
					ImGui::ProgressBar(videoMemoryMB / budgetMB, ImVec2(-1.0f, 0.0f));
				}
			}

			/// <summary>
			/// パネルの中身を描画する。
			/// </summary>
			void DrawProfilerPanel()
			{
				DrawFrameTime();

				ImGui::SeparatorText("CPU");
				DrawSections();

				ImGui::SeparatorText("Draw");
				DrawRenderStats();

				ImGui::SeparatorText("Objects");
				DrawGameObjectStats();

				ImGui::SeparatorText("Memory");
				DrawMemoryStats();

				ImGui::Separator();
				ImGui::TextDisabled("%s", U8("自分のコードを計測するには K2_PROFILE_SCOPE(\"名前\");"));
			}
		}

		void RegisterProfilerPanel()
		{
			DebugGui::RegisterPanel("Profiler", "Profiler", DrawProfilerPanel);
		}

#else // K2_ENABLE_DEBUG_GUI

		void RegisterProfilerPanel() {}

#endif // K2_ENABLE_DEBUG_GUI
	}
}
