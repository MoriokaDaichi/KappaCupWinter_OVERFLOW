/*!
 *@brief	デバッグGUI(Dear ImGui)。
 */

#pragma once

// デバッグ機能を組み込むかどうかの切り替え(K2_ENABLE_DEBUG_GUI)は
// dbg/DebugConfig.h にある。
#include "dbg/DebugConfig.h"

#include "imgui/imgui.h"
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace nsK2EngineLow {
	class RenderContext;

	/// <summary>
	/// ImGuiに渡す文字列をUTF-8に変換する。
	/// </summary>
	/// <remarks>
	/// ImGuiに渡す文字列は、UTF-8でなければならない。
	/// ところがVisual Studioで作ったソースファイルは、たいていShift-JISなので、
	/// 日本語をそのまま渡すと文字化けしてしまう。
	///
	///     ImGui::Text("敵の数 = %d", num);        // 文字化けする
	///     ImGui::Text(U8("敵の数 = %d"), num);    // 正しく表示される
	///
	/// 英数字だけの文字列や、すでにUTF-8になっている文字列は、
	/// そのまま素通しされるので、とりあえず付けておいても害は無い。
	///
	/// 戻り値は内部の一時バッファを指している。
	/// 長期間持ち回らず、その場でImGuiに渡すこと。
	/// </remarks>
	/// <param name="text">変換したい文字列。</param>
	/// <returns>UTF-8に変換された文字列。</returns>
	const char* U8(const char* text);

	/// <summary>
	/// デバッグGUI。
	/// </summary>
	/// <remarks>
	/// Dear ImGui を利用したデバッグ用のGUI。
	/// 起動後、F1キーで表示/非表示を切り替えることができる。
	///
	/// 【自分のデバッグパネルを追加する】
	///   DebugGui::RegisterPanel("Game", "Player", []() {
	///       ImGui::Text("HP = %d", hp);
	///       ImGui::SliderFloat("speed", &speed, 0.0f, 500.0f);
	///   });
	///
	/// 登録した関数の中身が、そのままウィンドウの中身になる。
	/// ImGui::Begin()/ImGui::End() を自分で書く必要はない。
	///
	/// 【ゲームオブジェクトの中から登録する場合】
	/// thisをキャプチャしたラムダ式を登録すると、
	/// そのゲームオブジェクトが死んだあともパネルが呼ばれ続けてしまい、
	/// 削除済みのメモリにアクセスしてクラッシュする。
	/// かならず DebugGuiPanel クラス(このファイルの下の方)を使うこと。
	/// </remarks>
	class DebugGui {
	public:
		/// <summary>
		/// パネルの中身を描画する関数の型。
		/// </summary>
		using PanelFunc = std::function<void()>;

		//---------------------------------------------------------------------
		// ここから下の4つはエンジンが呼び出す関数。
		// ゲーム側から呼び出す必要はない。
		//---------------------------------------------------------------------
		/// <summary>
		/// 初期化。
		/// </summary>
		/// <param name="hwnd">ウィンドウハンドル。</param>
		static void Init(HWND hwnd);
		/// <summary>
		/// 終了処理。
		/// </summary>
		static void Terminate();
		/// <summary>
		/// フレームの開始。
		/// </summary>
		static void BeginFrame();
		/// <summary>
		/// 描画。
		/// </summary>
		static void Render(RenderContext& rc);

		//---------------------------------------------------------------------
		// ここから下がゲーム側から使う関数。
		//---------------------------------------------------------------------
		/// <summary>
		/// デバッグパネルを登録する。
		/// </summary>
		/// <param name="category">
		/// メニューバーでの分類名。("Game"、"Graphics"など)
		/// </param>
		/// <param name="name">
		/// パネルの名前。ウィンドウのタイトルになる。
		/// すでに同じ名前で登録されている場合は、中身が差し替えられる。
		/// </param>
		/// <param name="drawFunc">パネルの中身を描画する関数。</param>
		/// <param name="isDefaultVisible">
		/// trueだと最初から表示された状態になる。
		/// </param>
		static void RegisterPanel(
			const char* category,
			const char* name,
			PanelFunc drawFunc,
			bool isDefaultVisible = false
		);
		/// <summary>
		/// デバッグパネルの登録を解除する。
		/// </summary>
		/// <param name="name">RegisterPanelで指定したパネルの名前。</param>
		static void UnregisterPanel(const char* name);

		/// <summary>
		/// デバッグGUI全体が表示されているか判定。
		/// </summary>
		static bool IsVisible();
		/// <summary>
		/// デバッグGUI全体の表示/非表示を設定する。
		/// </summary>
		static void SetVisible(bool isVisible);
		/// <summary>
		/// デバッグGUI全体の表示/非表示を切り替える。
		/// </summary>
		static void ToggleVisible();

		/// <summary>
		/// 指定したパネルが表示されているか判定。
		/// </summary>
		static bool IsPanelVisible(const char* name);
		/// <summary>
		/// 指定したパネルの表示/非表示を設定する。
		/// </summary>
		static void SetPanelVisible(const char* name, bool isVisible);

		/// <summary>
		/// ImGuiがマウス入力を使用中か判定。
		/// </summary>
		/// <remarks>
		/// trueが返ってきているときは、GUIの上でマウスが操作されている。
		/// ゲーム側のマウス処理を止めたい場合はこれを見ること。
		/// </remarks>
		static bool WantCaptureMouse();
		/// <summary>
		/// ImGuiがキーボード入力を使用中か判定。
		/// </summary>
		static bool WantCaptureKeyboard();

		/// <summary>
		/// デバッグGUIが利用可能か判定。
		/// </summary>
		/// <remarks>
		/// K2_ENABLE_DEBUG_GUIが無効な場合と、
		/// 初期化に失敗している場合はfalseが返ってくる。
		/// </remarks>
		static bool IsAvailable();
	};

	/// <summary>
	/// デバッグパネルの登録をお世話してくれるクラス。
	/// </summary>
	/// <remarks>
	/// このクラスのインスタンスをメンバ変数として持っておくと、
	/// インスタンスが破棄されるタイミングで自動的に登録が解除される。
	///
	/// 【使い方】
	///   class Player : public IGameObject {
	///       ...
	///   private:
	///       DebugGuiPanel m_debugPanel;    // メンバ変数として持つ
	///   };
	///
	///   bool Player::Start()
	///   {
	///       m_debugPanel.Register("Game", "Player", [this]() {
	///           ImGui::Text("HP = %d", m_hp);
	///       });
	///       return true;
	///   }
	///
	/// こうしておけば、Playerが死んだときにパネルの登録も自動で消える。
	/// (これを「RAII」と呼ぶ。C++では非常によく使われる考え方)
	/// </remarks>
	class DebugGuiPanel {
	public:
		DebugGuiPanel() = default;
		/// <summary>
		/// デストラクタ。登録を自動的に解除する。
		/// </summary>
		~DebugGuiPanel()
		{
			Unregister();
		}
		// コピーされると解除が二重に走ってしまうので禁止する。
		DebugGuiPanel(const DebugGuiPanel&) = delete;
		DebugGuiPanel& operator=(const DebugGuiPanel&) = delete;

		/// <summary>
		/// パネルを登録する。
		/// </summary>
		void Register(
			const char* category,
			const char* name,
			DebugGui::PanelFunc drawFunc,
			bool isDefaultVisible = false
		)
		{
			Unregister();
			m_name = name;
			DebugGui::RegisterPanel(category, name, drawFunc, isDefaultVisible);
		}
		/// <summary>
		/// パネルの登録を解除する。
		/// </summary>
		void Unregister()
		{
			if (m_name.empty()) {
				return;
			}
			DebugGui::UnregisterPanel(m_name.c_str());
			m_name.clear();
		}
	private:
		std::string m_name;		// 登録中のパネル名。空なら未登録。
	};
}
