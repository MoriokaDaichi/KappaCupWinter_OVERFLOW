/*!
 *@brief	デバッグGUI(Dear ImGui)。
 */

#include "k2EngineLowPreCompile.h"
#include "DebugGui.h"
#include "graphics/GraphicsEngine.h"

#ifdef K2_ENABLE_DEBUG_GUI
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx12.h"
#include <algorithm>

// ImGuiがウィンドウメッセージを処理するための関数。
// imgui_impl_win32.h では宣言が #if 0 で囲まれているので、
// 使う側が宣言をコピーしてくるお作法になっている。
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
	HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif // K2_ENABLE_DEBUG_GUI

namespace nsK2EngineLow {

	const char* U8(const char* text)
	{
		if (text == nullptr) {
			return "";
		}
		// すでにUTF-8として解釈できるならそのまま返す。
		// (英数字だけの文字列もここを通る)
		if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text, -1, nullptr, 0) > 0) {
			return text;
		}
		// Shift-JIS(CP932)とみなしてUTF-8に変換する。
		// 1回の呼び出しで複数の文字列を変換したいことがあるので、
		// バッファを何個か用意して順番に使い回している。
		const int BUFFER_NUM = 8;
		const int BUFFER_SIZE = 1024;
		static char buffers[BUFFER_NUM][BUFFER_SIZE];
		static int bufferIndex = 0;
		char* buffer = buffers[bufferIndex];
		bufferIndex = (bufferIndex + 1) % BUFFER_NUM;

		wchar_t wideText[BUFFER_SIZE];
		int wideLength = MultiByteToWideChar(
			CP_ACP, 0, text, -1, wideText, BUFFER_SIZE);
		if (wideLength <= 0) {
			return text;
		}
		if (WideCharToMultiByte(
				CP_UTF8, 0, wideText, -1, buffer, BUFFER_SIZE, nullptr, nullptr) <= 0
		) {
			return text;
		}
		return buffer;
	}

#ifdef K2_ENABLE_DEBUG_GUI

	namespace {
		/// <summary>
		/// ImGuiがテクスチャを登録するためのディスクリプタヒープ。
		/// </summary>
		/// <remarks>
		/// ImGuiはフォントや画像を描画するために、SRVのディスクリプタを必要とする。
		/// エンジン本体が使っているディスクリプタヒープと混ぜるとややこしくなるので、
		/// ImGui専用のヒープをここで独立して用意している。
		/// </remarks>
		class SrvDescriptorHeap {
		public:
			/// <summary>
			/// ヒープを作成する。
			/// </summary>
			bool Create(ID3D12Device* device, int numDescriptors)
			{
				D3D12_DESCRIPTOR_HEAP_DESC desc = {};
				desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
				desc.NumDescriptors = numDescriptors;
				desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
				if (FAILED(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_heap)))) {
					return false;
				}
				m_descriptorSize = device->GetDescriptorHandleIncrementSize(desc.Type);
				m_cpuStart = m_heap->GetCPUDescriptorHandleForHeapStart();
				m_gpuStart = m_heap->GetGPUDescriptorHandleForHeapStart();

				// 空きスロットの一覧を作る。後ろから取り出したいので逆順に積んでおく。
				m_freeIndices.clear();
				m_freeIndices.reserve(numDescriptors);
				for (int i = numDescriptors - 1; i >= 0; i--) {
					m_freeIndices.push_back(i);
				}
				return true;
			}
			/// <summary>
			/// ヒープを破棄する。
			/// </summary>
			void Destroy()
			{
				if (m_heap != nullptr) {
					m_heap->Release();
					m_heap = nullptr;
				}
				m_freeIndices.clear();
			}
			/// <summary>
			/// ディスクリプタを1つ確保する。
			/// </summary>
			void Alloc(
				D3D12_CPU_DESCRIPTOR_HANDLE* outCpuHandle,
				D3D12_GPU_DESCRIPTOR_HANDLE* outGpuHandle)
			{
				if (m_freeIndices.empty()) {
					// ディスクリプタが足りない。SRV_DESCRIPTOR_NUMを増やすこと。
					outCpuHandle->ptr = 0;
					outGpuHandle->ptr = 0;
					return;
				}
				int index = m_freeIndices.back();
				m_freeIndices.pop_back();
				outCpuHandle->ptr = m_cpuStart.ptr + (SIZE_T)index * m_descriptorSize;
				outGpuHandle->ptr = m_gpuStart.ptr + (UINT64)index * m_descriptorSize;
			}
			/// <summary>
			/// ディスクリプタを解放する。
			/// </summary>
			void Free(
				D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle,
				D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle)
			{
				(void)gpuHandle;
				if (m_heap == nullptr || m_descriptorSize == 0) {
					return;
				}
				int index = (int)((cpuHandle.ptr - m_cpuStart.ptr) / m_descriptorSize);
				m_freeIndices.push_back(index);
			}
			ID3D12DescriptorHeap* GetHeap() const
			{
				return m_heap;
			}
		private:
			ID3D12DescriptorHeap* m_heap = nullptr;			// ディスクリプタヒープ。
			UINT m_descriptorSize = 0;						// ディスクリプタ1つあたりのサイズ。
			D3D12_CPU_DESCRIPTOR_HANDLE m_cpuStart = {};	// CPU側の先頭ハンドル。
			D3D12_GPU_DESCRIPTOR_HANDLE m_gpuStart = {};	// GPU側の先頭ハンドル。
			std::vector<int> m_freeIndices;					// 空いているスロットの番号。
		};

		/// <summary>
		/// デバッグパネル1枚分の情報。
		/// </summary>
		struct Panel {
			std::string category;			// メニューバーでの分類名。
			std::string name;				// パネル名。ウィンドウのタイトルになる。
			DebugGui::PanelFunc drawFunc;	// 中身を描画する関数。
			bool isVisible = false;			// 表示中か。
			bool isRemoved = false;			// 登録解除されたか。
		};

		// ImGuiに渡すディスクリプタの数。
		// 画像をたくさんデバッグ表示したくなったらここを増やす。
		const int SRV_DESCRIPTOR_NUM = 64;

		SrvDescriptorHeap	g_srvDescriptorHeap;			// ImGui用のディスクリプタヒープ。
		bool				g_isInitialized = false;		// 初期化済みか。
		bool				g_isVisible = true;				// デバッグGUI全体を表示するか。
		bool				g_isShowImGuiDemo = false;		// ImGuiのデモウィンドウを表示するか。
		HWND				g_hwnd = nullptr;				// ウィンドウハンドル。
		WNDPROC				g_originalWndProc = nullptr;	// 元のウィンドウプロシージャ。
		std::vector<std::unique_ptr<Panel>> g_panels;		// 登録されているパネル。

		/// <summary>
		/// 名前からパネルを探す。
		/// </summary>
		Panel* FindPanel(const char* name)
		{
			for (auto& panel : g_panels) {
				if (panel->isRemoved == false && panel->name == name) {
					return panel.get();
				}
			}
			return nullptr;
		}

		/// <summary>
		/// ImGuiがSRVのディスクリプタを要求してきたときに呼ばれる。
		/// </summary>
		void SrvDescriptorAllocFunc(
			ImGui_ImplDX12_InitInfo* info,
			D3D12_CPU_DESCRIPTOR_HANDLE* outCpuHandle,
			D3D12_GPU_DESCRIPTOR_HANDLE* outGpuHandle)
		{
			auto heap = reinterpret_cast<SrvDescriptorHeap*>(info->UserData);
			heap->Alloc(outCpuHandle, outGpuHandle);
		}
		/// <summary>
		/// ImGuiがSRVのディスクリプタを返却してきたときに呼ばれる。
		/// </summary>
		void SrvDescriptorFreeFunc(
			ImGui_ImplDX12_InitInfo* info,
			D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle,
			D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle)
		{
			auto heap = reinterpret_cast<SrvDescriptorHeap*>(info->UserData);
			heap->Free(cpuHandle, gpuHandle);
		}

		/// <summary>
		/// 差し替えたウィンドウプロシージャ。
		/// </summary>
		/// <remarks>
		/// ImGuiはマウスやキーボードの入力をウィンドウメッセージから受け取る。
		/// Game側のsystem.cppを書き換えなくてもよいように、
		/// エンジン側でウィンドウプロシージャを差し替えて横取りしている。
		/// </remarks>
		LRESULT CALLBACK DebugGuiWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
		{
			if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam)) {
				return 1;
			}
			if (g_originalWndProc != nullptr) {
				return CallWindowProc(g_originalWndProc, hwnd, msg, wParam, lParam);
			}
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}

		/// <summary>
		/// GPUの処理がすべて終わるのを待つ。
		/// </summary>
		/// <remarks>
		/// ImGuiが確保したテクスチャなどをGPUが使っている最中に解放すると
		/// クラッシュするので、終了処理の前に待ち合わせを行う。
		/// </remarks>
		void WaitForGpuIdle()
		{
			auto device = g_graphicsEngine->GetD3DDevice();
			auto commandQueue = g_graphicsEngine->GetCommandQueue();
			if (device == nullptr || commandQueue == nullptr) {
				return;
			}
			ID3D12Fence* fence = nullptr;
			if (FAILED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)))) {
				return;
			}
			HANDLE fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			if (fenceEvent != nullptr) {
				commandQueue->Signal(fence, 1);
				fence->SetEventOnCompletion(1, fenceEvent);
				WaitForSingleObject(fenceEvent, INFINITE);
				CloseHandle(fenceEvent);
			}
			fence->Release();
		}

		/// <summary>
		/// 日本語が表示できるフォントを読み込む。
		/// </summary>
		void LoadJapaneseFont()
		{
			// Windowsに標準で入っているフォントを順番に探す。
			const char* fontFilePathList[] = {
				"C:/Windows/Fonts/meiryo.ttc",		// メイリオ
				"C:/Windows/Fonts/YuGothM.ttc",		// 游ゴシック Medium
				"C:/Windows/Fonts/msgothic.ttc",	// MS ゴシック
			};
			auto& io = ImGui::GetIO();
			for (auto fontFilePath : fontFilePathList) {
				if (GetFileAttributesA(fontFilePath) == INVALID_FILE_ATTRIBUTES) {
					continue;
				}
				if (io.Fonts->AddFontFromFileTTF(fontFilePath, 18.0f) != nullptr) {
					// 読み込めた。
					// ImGui 1.92以降は、必要になった文字だけを動的に
					// テクスチャに焼いてくれるので、文字範囲の指定は不要。
					return;
				}
			}
			// 見つからなかったので、ImGuiの組み込みフォントを使う。
			// (この場合、日本語は表示できない)
			io.Fonts->AddFontDefault();
		}

		/// <summary>
		/// メニューバーを描画する。
		/// </summary>
		void DrawMainMenuBar()
		{
			if (ImGui::BeginMainMenuBar() == false) {
				return;
			}
			if (ImGui::BeginMenu("Debug")) {
				if (ImGui::MenuItem("Close All Panels")) {
					for (auto& panel : g_panels) {
						panel->isVisible = false;
					}
				}
				ImGui::Separator();
				ImGui::MenuItem("ImGui Demo", nullptr, &g_isShowImGuiDemo);
				ImGui::Separator();
				if (ImGui::MenuItem("Hide Debug GUI", "F1")) {
					g_isVisible = false;
				}
				ImGui::EndMenu();
			}

			// カテゴリごとにメニューを作る。
			// 登録された順番を保ちたいので、std::mapではなくvectorで処理している。
			std::vector<const std::string*> categories;
			for (auto& panel : g_panels) {
				if (panel->isRemoved) {
					continue;
				}
				bool isFound = false;
				for (auto category : categories) {
					if (*category == panel->category) {
						isFound = true;
						break;
					}
				}
				if (isFound == false) {
					categories.push_back(&panel->category);
				}
			}
			for (auto category : categories) {
				if (ImGui::BeginMenu(category->c_str()) == false) {
					continue;
				}
				for (auto& panel : g_panels) {
					if (panel->isRemoved || panel->category != *category) {
						continue;
					}
					ImGui::MenuItem(panel->name.c_str(), nullptr, &panel->isVisible);
				}
				ImGui::EndMenu();
			}

			// 右端にフレームレートを表示する。
			const auto& io = ImGui::GetIO();
			char frameRateText[64];
			sprintf_s(
				frameRateText,
				"%.1f FPS (%.2f ms)",
				io.Framerate,
				io.Framerate > 0.0f ? 1000.0f / io.Framerate : 0.0f
			);
			float textWidth = ImGui::CalcTextSize(frameRateText).x;
			ImGui::SameLine(ImGui::GetWindowWidth() - textWidth - 20.0f);
			ImGui::TextUnformatted(frameRateText);

			ImGui::EndMainMenuBar();
		}

		/// <summary>
		/// 登録されているパネルをすべて描画する。
		/// </summary>
		void DrawPanels()
		{
			// 描画中にパネルが追加されることがあるので、
			// 範囲for文ではなく添え字で回している。
			for (size_t i = 0; i < g_panels.size(); i++) {
				Panel* panel = g_panels[i].get();
				if (panel->isRemoved || panel->isVisible == false) {
					continue;
				}
				ImGui::SetNextWindowSize(ImVec2(400.0f, 560.0f), ImGuiCond_FirstUseEver);
				if (ImGui::Begin(panel->name.c_str(), &panel->isVisible)) {
					if (panel->drawFunc) {
						panel->drawFunc();
					}
				}
				ImGui::End();
			}
		}
	}

	void DebugGui::Init(HWND hwnd)
	{
		if (g_isInitialized) {
			return;
		}
		if (g_graphicsEngine == nullptr) {
			return;
		}
		auto device = g_graphicsEngine->GetD3DDevice();
		if (device == nullptr) {
			return;
		}
		// ImGui専用のディスクリプタヒープを作成。
		if (g_srvDescriptorHeap.Create(device, SRV_DESCRIPTOR_NUM) == false) {
			return;
		}

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		auto& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		// ウィンドウの位置や大きさはimgui.iniに自動的に保存される。
		io.IniFilename = "imgui.ini";

		ImGui::StyleColorsDark();
		LoadJapaneseFont();

		ImGui_ImplWin32_Init(hwnd);

		ImGui_ImplDX12_InitInfo initInfo = {};
		initInfo.Device = device;
		initInfo.CommandQueue = g_graphicsEngine->GetCommandQueue();
		initInfo.NumFramesInFlight = 2;						// フレームバッファの数。
		initInfo.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;	// フレームバッファのフォーマット。
		initInfo.DSVFormat = DXGI_FORMAT_D32_FLOAT;			// デプスバッファのフォーマット。
		initInfo.SrvDescriptorHeap = g_srvDescriptorHeap.GetHeap();
		initInfo.SrvDescriptorAllocFn = SrvDescriptorAllocFunc;
		initInfo.SrvDescriptorFreeFn = SrvDescriptorFreeFunc;
		initInfo.UserData = &g_srvDescriptorHeap;
		ImGui_ImplDX12_Init(&initInfo);

		// ウィンドウプロシージャを差し替えて、入力を横取りする。
		g_hwnd = hwnd;
		g_originalWndProc = reinterpret_cast<WNDPROC>(
			SetWindowLongPtr(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(DebugGuiWndProc))
		);

		g_isInitialized = true;
	}

	void DebugGui::Terminate()
	{
		if (g_isInitialized == false) {
			return;
		}
		// GPUが使い終わるのを待ってから解放する。
		WaitForGpuIdle();

		// ウィンドウプロシージャを元に戻す。
		if (g_hwnd != nullptr && g_originalWndProc != nullptr) {
			SetWindowLongPtr(g_hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(g_originalWndProc));
			g_originalWndProc = nullptr;
			g_hwnd = nullptr;
		}
		ImGui_ImplDX12_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
		g_srvDescriptorHeap.Destroy();
		g_panels.clear();
		g_isInitialized = false;
	}

	void DebugGui::BeginFrame()
	{
		if (g_isInitialized == false) {
			return;
		}
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		// F1キーで表示/非表示を切り替える。
		// テキストを入力している最中は反応させない。
		if (ImGui::GetIO().WantTextInput == false
			&& ImGui::IsKeyPressed(ImGuiKey_F1, false)
		) {
			g_isVisible = !g_isVisible;
		}
	}

	void DebugGui::Render(RenderContext& rc)
	{
		if (g_isInitialized == false) {
			return;
		}
		// 登録解除されたパネルをここで本当に取り除く。
		// (描画中に取り除くと、実行中の関数を消してしまうことになるため)
		g_panels.erase(
			std::remove_if(
				g_panels.begin(),
				g_panels.end(),
				[](const std::unique_ptr<Panel>& panel) { return panel->isRemoved; }
			),
			g_panels.end()
		);

		if (g_isVisible) {
			DrawMainMenuBar();
			DrawPanels();
			if (g_isShowImGuiDemo) {
				ImGui::ShowDemoWindow(&g_isShowImGuiDemo);
			}
		}

		// 非表示のときもImGui::Render()は必ず呼ぶこと。
		// NewFrame()とRender()は必ずペアで呼び出す必要がある。
		ImGui::Render();

		// フレームバッファに対して描画する。
		g_graphicsEngine->ChangeRenderTargetToFrameBuffer(rc);
		ImGui_ImplDX12_RenderDrawData(
			ImGui::GetDrawData(),
			g_graphicsEngine->GetCommandList()
		);
	}

	void DebugGui::RegisterPanel(
		const char* category,
		const char* name,
		PanelFunc drawFunc,
		bool isDefaultVisible)
	{
		if (g_isInitialized == false) {
			return;
		}
		// すでに同じ名前で登録されていたら中身を差し替える。
		Panel* panel = FindPanel(name);
		if (panel != nullptr) {
			panel->category = category;
			panel->drawFunc = drawFunc;
			return;
		}
		std::unique_ptr<Panel> newPanel(new Panel);
		newPanel->category = category;
		newPanel->name = name;
		newPanel->drawFunc = drawFunc;
		newPanel->isVisible = isDefaultVisible;
		g_panels.push_back(std::move(newPanel));
	}

	void DebugGui::UnregisterPanel(const char* name)
	{
		Panel* panel = FindPanel(name);
		if (panel != nullptr) {
			// ここではフラグを立てるだけ。実際に取り除くのはRender()の先頭。
			panel->isRemoved = true;
		}
	}

	bool DebugGui::IsVisible()
	{
		return g_isInitialized && g_isVisible;
	}

	void DebugGui::SetVisible(bool isVisible)
	{
		g_isVisible = isVisible;
	}

	void DebugGui::ToggleVisible()
	{
		g_isVisible = !g_isVisible;
	}

	bool DebugGui::IsPanelVisible(const char* name)
	{
		Panel* panel = FindPanel(name);
		return panel != nullptr && panel->isVisible;
	}

	void DebugGui::SetPanelVisible(const char* name, bool isVisible)
	{
		Panel* panel = FindPanel(name);
		if (panel != nullptr) {
			panel->isVisible = isVisible;
		}
	}

	bool DebugGui::WantCaptureMouse()
	{
		if (g_isInitialized == false || g_isVisible == false) {
			return false;
		}
		return ImGui::GetIO().WantCaptureMouse;
	}

	bool DebugGui::WantCaptureKeyboard()
	{
		if (g_isInitialized == false || g_isVisible == false) {
			return false;
		}
		return ImGui::GetIO().WantCaptureKeyboard;
	}

	bool DebugGui::IsAvailable()
	{
		return g_isInitialized;
	}

#else // K2_ENABLE_DEBUG_GUI

	// デバッグGUIが無効なときは、すべて何もしない関数になる。
	void DebugGui::Init(HWND) {}
	void DebugGui::Terminate() {}
	void DebugGui::BeginFrame() {}
	void DebugGui::Render(RenderContext&) {}
	void DebugGui::RegisterPanel(const char*, const char*, PanelFunc, bool) {}
	void DebugGui::UnregisterPanel(const char*) {}
	bool DebugGui::IsVisible() { return false; }
	void DebugGui::SetVisible(bool) {}
	void DebugGui::ToggleVisible() {}
	bool DebugGui::IsPanelVisible(const char*) { return false; }
	void DebugGui::SetPanelVisible(const char*, bool) {}
	bool DebugGui::WantCaptureMouse() { return false; }
	bool DebugGui::WantCaptureKeyboard() { return false; }
	bool DebugGui::IsAvailable() { return false; }

#endif // K2_ENABLE_DEBUG_GUI
}
