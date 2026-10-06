/*!
 *@brief	簡易プロファイラ。
 */

#include "k2EngineLowPreCompile.h"
#include "DebugProfiler.h"

#ifdef K2_ENABLE_DEBUG_GUI
#include <dxgi1_4.h>
#include <psapi.h>
#pragma comment(lib, "psapi.lib")
#endif // K2_ENABLE_DEBUG_GUI

namespace nsK2EngineLow {
	namespace nsDbg {

#ifdef K2_ENABLE_DEBUG_GUI

		int g_drawCallCount = 0;
		int g_drawInstanceCount = 0;

		namespace {
			// フレーム時間の履歴の数。グラフの横幅。
			const int FRAME_TIME_HISTORY_NUM = 180;
			// 表示を更新する間隔(秒)。
			// 毎フレーム更新すると数字がチカチカして読めないので、
			// この間隔で平均を取って表示する。
			const float DISPLAY_UPDATE_INTERVAL = 0.25f;

			/// <summary>
			/// 計測中の区間。
			/// </summary>
			struct RunningSection {
				int resultIndex = 0;			// g_frameResults のどこに書き込むか。
				LARGE_INTEGER beginTime = {};
			};

			LARGE_INTEGER	g_performanceFrequency = {};	// 1秒あたりのカウント数。
			LARGE_INTEGER	g_frameBeginTime = {};			// このフレームの開始時刻。
			bool			g_isFirstFrame = true;			// 最初のフレームか。

			float			g_frameTimeMs = 0.0f;						// 直近のフレーム時間。
			float			g_frameTimeHistory[FRAME_TIME_HISTORY_NUM] = {};	// フレーム時間の履歴。
			int				g_frameTimeHistoryIndex = 0;				// 履歴の書き込み位置。

			std::vector<RunningSection>	g_runningSections;	// 計測中の区間のスタック。
			std::vector<DebugProfiler::SectionResult> g_frameResults;	// このフレームの結果。

			// 表示用(平均を取ったもの)。
			std::vector<DebugProfiler::SectionResult> g_displayResults;
			std::vector<float>	g_accumulatedTimes;		// 平均を取るための合計。
			int					g_accumulatedFrameNum = 0;
			float				g_accumulatedSecond = 0.0f;
			int					g_displayDrawCallCount = 0;
			int					g_displayDrawInstanceCount = 0;

			// メモリ使用量。重いので表示更新のタイミングでだけ取得する。
			float			g_processMemoryMB = 0.0f;
			float			g_videoMemoryMB = -1.0f;
			float			g_videoMemoryBudgetMB = -1.0f;
			IDXGIAdapter3*	g_dxgiAdapter = nullptr;
			bool			g_isDxgiAdapterInitialized = false;

			/// <summary>
			/// ビデオメモリの量を調べるためのアダプタを取得する。
			/// </summary>
			IDXGIAdapter3* GetDxgiAdapter()
			{
				if (g_isDxgiAdapterInitialized) {
					return g_dxgiAdapter;
				}
				g_isDxgiAdapterInitialized = true;

				if (g_graphicsEngine == nullptr) {
					return nullptr;
				}
				auto device = g_graphicsEngine->GetD3DDevice();
				if (device == nullptr) {
					return nullptr;
				}
				IDXGIFactory4* factory = nullptr;
				if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory)))) {
					return nullptr;
				}
				IDXGIAdapter1* adapter1 = nullptr;
				if (SUCCEEDED(factory->EnumAdapterByLuid(
						device->GetAdapterLuid(), IID_PPV_ARGS(&adapter1)))
				) {
					adapter1->QueryInterface(IID_PPV_ARGS(&g_dxgiAdapter));
					adapter1->Release();
				}
				factory->Release();
				return g_dxgiAdapter;
			}

			/// <summary>
			/// メモリの使用量を調べる。
			/// </summary>
			void UpdateMemoryUsage()
			{
				const float BYTE_TO_MB = 1.0f / (1024.0f * 1024.0f);

				PROCESS_MEMORY_COUNTERS memoryCounters = {};
				if (GetProcessMemoryInfo(
						GetCurrentProcess(), &memoryCounters, sizeof(memoryCounters))
				) {
					g_processMemoryMB = (float)memoryCounters.WorkingSetSize * BYTE_TO_MB;
				}

				auto adapter = GetDxgiAdapter();
				if (adapter != nullptr) {
					DXGI_QUERY_VIDEO_MEMORY_INFO videoMemoryInfo = {};
					if (SUCCEEDED(adapter->QueryVideoMemoryInfo(
							0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &videoMemoryInfo))
					) {
						g_videoMemoryMB = (float)videoMemoryInfo.CurrentUsage * BYTE_TO_MB;
						g_videoMemoryBudgetMB = (float)videoMemoryInfo.Budget * BYTE_TO_MB;
					}
				}
			}

			/// <summary>
			/// 表示用の平均値を更新する。
			/// </summary>
			void UpdateDisplayResults(float frameTimeSecond)
			{
				// 区間の構成が変わったら、それまでの合計は捨てる。
				if (g_accumulatedTimes.size() != g_frameResults.size()) {
					g_accumulatedTimes.assign(g_frameResults.size(), 0.0f);
					g_accumulatedFrameNum = 0;
					g_accumulatedSecond = 0.0f;
				}
				for (size_t i = 0; i < g_frameResults.size(); i++) {
					g_accumulatedTimes[i] += g_frameResults[i].timeMs;
				}
				g_accumulatedFrameNum++;
				g_accumulatedSecond += frameTimeSecond;

				if (g_accumulatedSecond < DISPLAY_UPDATE_INTERVAL) {
					return;
				}
				// 平均を取って表示用に反映する。
				g_displayResults = g_frameResults;
				for (size_t i = 0; i < g_displayResults.size(); i++) {
					g_displayResults[i].timeMs =
						g_accumulatedTimes[i] / (float)g_accumulatedFrameNum;
				}
				g_displayDrawCallCount = g_drawCallCount;
				g_displayDrawInstanceCount = g_drawInstanceCount;

				g_accumulatedTimes.assign(g_frameResults.size(), 0.0f);
				g_accumulatedFrameNum = 0;
				g_accumulatedSecond = 0.0f;

				UpdateMemoryUsage();
			}
		}

		void DebugProfiler::BeginFrame()
		{
			if (g_performanceFrequency.QuadPart == 0) {
				QueryPerformanceFrequency(&g_performanceFrequency);
			}
			LARGE_INTEGER now;
			QueryPerformanceCounter(&now);

			if (g_isFirstFrame) {
				g_isFirstFrame = false;
			}
			else {
				// 前のフレームの開始からの時間が、1フレームにかかった時間。
				double elapsed =
					(double)(now.QuadPart - g_frameBeginTime.QuadPart)
					/ (double)g_performanceFrequency.QuadPart;
				g_frameTimeMs = (float)(elapsed * 1000.0);
				g_frameTimeHistory[g_frameTimeHistoryIndex] = g_frameTimeMs;
				g_frameTimeHistoryIndex =
					(g_frameTimeHistoryIndex + 1) % FRAME_TIME_HISTORY_NUM;
			}
			g_frameBeginTime = now;

			// このフレームの計測結果をリセット。
			g_runningSections.clear();
			g_frameResults.clear();
			g_drawCallCount = 0;
			g_drawInstanceCount = 0;
		}

		void DebugProfiler::EndFrame()
		{
			// 閉じ忘れている区間があれば、ここで閉じる。
			while (g_runningSections.empty() == false) {
				EndSection();
			}
			UpdateDisplayResults(g_frameTimeMs / 1000.0f);
		}

		void DebugProfiler::Terminate()
		{
			if (g_dxgiAdapter != nullptr) {
				g_dxgiAdapter->Release();
				g_dxgiAdapter = nullptr;
			}
			g_isDxgiAdapterInitialized = false;
		}

		void DebugProfiler::BeginSection(const char* name)
		{
			// 結果の並び順を「計測を開始した順」にしたいので、
			// この時点で書き込む場所を確保しておく。
			SectionResult result;
			result.name = name;
			result.depth = (int)g_runningSections.size();
			result.timeMs = 0.0f;
			g_frameResults.push_back(result);

			RunningSection section;
			section.resultIndex = (int)g_frameResults.size() - 1;
			QueryPerformanceCounter(&section.beginTime);
			g_runningSections.push_back(section);
		}

		void DebugProfiler::EndSection()
		{
			if (g_runningSections.empty()) {
				return;
			}
			const RunningSection& section = g_runningSections.back();

			LARGE_INTEGER now;
			QueryPerformanceCounter(&now);
			double elapsed =
				(double)(now.QuadPart - section.beginTime.QuadPart)
				/ (double)g_performanceFrequency.QuadPart;

			// BeginSectionで確保しておいた場所に書き込む。
			g_frameResults[section.resultIndex].timeMs = (float)(elapsed * 1000.0);
			g_runningSections.pop_back();
		}

		int DebugProfiler::GetSectionResultNum()
		{
			return (int)g_displayResults.size();
		}

		const DebugProfiler::SectionResult& DebugProfiler::GetSectionResult(int index)
		{
			static SectionResult dummy;
			if (index < 0 || index >= (int)g_displayResults.size()) {
				return dummy;
			}
			return g_displayResults[index];
		}

		float DebugProfiler::GetFrameTimeMs()
		{
			return g_frameTimeMs;
		}

		const float* DebugProfiler::GetFrameTimeHistory()
		{
			return g_frameTimeHistory;
		}

		int DebugProfiler::GetFrameTimeHistoryNum()
		{
			return FRAME_TIME_HISTORY_NUM;
		}

		float DebugProfiler::GetFrameTimeMsMax()
		{
			float maxTime = 0.0f;
			for (float time : g_frameTimeHistory) {
				maxTime = (std::max)(maxTime, time);
			}
			return maxTime;
		}

		int DebugProfiler::GetDrawCallCount()
		{
			return g_displayDrawCallCount;
		}

		int DebugProfiler::GetDrawInstanceCount()
		{
			return g_displayDrawInstanceCount;
		}

		float DebugProfiler::GetProcessMemoryMB()
		{
			return g_processMemoryMB;
		}

		float DebugProfiler::GetVideoMemoryMB()
		{
			return g_videoMemoryMB;
		}

		float DebugProfiler::GetVideoMemoryBudgetMB()
		{
			return g_videoMemoryBudgetMB;
		}

#else // K2_ENABLE_DEBUG_GUI

		void DebugProfiler::BeginFrame() {}
		void DebugProfiler::EndFrame() {}
		void DebugProfiler::Terminate() {}
		void DebugProfiler::BeginSection(const char*) {}
		void DebugProfiler::EndSection() {}
		int DebugProfiler::GetSectionResultNum() { return 0; }
		const DebugProfiler::SectionResult& DebugProfiler::GetSectionResult(int)
		{
			static SectionResult dummy;
			return dummy;
		}
		float DebugProfiler::GetFrameTimeMs() { return 0.0f; }
		const float* DebugProfiler::GetFrameTimeHistory() { return nullptr; }
		int DebugProfiler::GetFrameTimeHistoryNum() { return 0; }
		float DebugProfiler::GetFrameTimeMsMax() { return 0.0f; }
		int DebugProfiler::GetDrawCallCount() { return 0; }
		int DebugProfiler::GetDrawInstanceCount() { return 0; }
		float DebugProfiler::GetProcessMemoryMB() { return 0.0f; }
		float DebugProfiler::GetVideoMemoryMB() { return -1.0f; }
		float DebugProfiler::GetVideoMemoryBudgetMB() { return -1.0f; }

#endif // K2_ENABLE_DEBUG_GUI
	}
}
