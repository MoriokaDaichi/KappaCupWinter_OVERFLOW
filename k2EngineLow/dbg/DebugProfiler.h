/*!
 *@brief	簡易プロファイラ。
 */

#pragma once

#include "dbg/DebugConfig.h"

namespace nsK2EngineLow {
	namespace nsDbg {

#ifdef K2_ENABLE_DEBUG_GUI
		// このフレームのドローコール数。RenderContextから加算される。
		extern int g_drawCallCount;
		// このフレームに描画されたインスタンスの数。
		extern int g_drawInstanceCount;

		/// <summary>
		/// ドローコールを1回数える。
		/// </summary>
		/// <remarks>
		/// RenderContextの描画関数から呼び出している。
		/// デバッグ機能が無効なときは、何もしないコードに置き換わる。
		/// </remarks>
#define K2_COUNT_DRAW_CALL(numInstance)									\
		do {															\
			nsK2EngineLow::nsDbg::g_drawCallCount++;					\
			nsK2EngineLow::nsDbg::g_drawInstanceCount += (int)(numInstance);	\
		} while (0)
#else
#define K2_COUNT_DRAW_CALL(numInstance) ((void)0)
#endif

		/// <summary>
		/// 簡易プロファイラ。
		/// </summary>
		/// <remarks>
		/// 「この処理に何ミリ秒かかっているか」を計測して、
		/// Profilerパネルに表示するためのクラス。
		///
		/// 【自分のコードを計測する】
		///     void Player::Update()
		///     {
		///         K2_PROFILE_SCOPE("Player");
		///         ...
		///     }
		///
		/// この1行を書くだけで、その { } を抜けるまでの時間が計測される。
		/// 入れ子にすることもできる。
		///
		/// 計測は1フレームにつき数十個までを想定した簡易的なもの。
		/// 毎フレーム何千回も呼ばれる関数に入れると、計測自体が重くなるので注意。
		/// </remarks>
		class DebugProfiler {
		public:
			/// <summary>
			/// 1つの計測結果。
			/// </summary>
			struct SectionResult {
				const char* name = nullptr;	// 区間の名前。
				int depth = 0;				// 入れ子の深さ。
				float timeMs = 0.0f;		// かかった時間(ミリ秒)。
			};

			//-----------------------------------------------------------------
			// ここから下の3つはエンジンが呼び出す。
			//-----------------------------------------------------------------
			/// <summary>
			/// フレームの開始。
			/// </summary>
			static void BeginFrame();
			/// <summary>
			/// フレームの終了。
			/// </summary>
			static void EndFrame();
			/// <summary>
			/// 終了処理。
			/// </summary>
			static void Terminate();

			//-----------------------------------------------------------------
			// 計測。K2_PROFILE_SCOPE を使うほうが安全なので、
			// 基本的にはそちらを使うこと。
			//-----------------------------------------------------------------
			/// <summary>
			/// 計測を開始する。
			/// </summary>
			/// <param name="name">
			/// 区間の名前。文字列リテラルを渡すこと。
			/// (アドレスをそのまま覚えているので、一時的な文字列を渡してはいけない)
			/// </param>
			static void BeginSection(const char* name);
			/// <summary>
			/// 計測を終了する。
			/// </summary>
			static void EndSection();

			//-----------------------------------------------------------------
			// 結果の取得。
			//-----------------------------------------------------------------
			/// <summary>
			/// 計測結果の数を取得。
			/// </summary>
			static int GetSectionResultNum();
			/// <summary>
			/// 計測結果を取得。
			/// </summary>
			static const SectionResult& GetSectionResult(int index);
			/// <summary>
			/// 直近のフレームにかかった時間(ミリ秒)。
			/// </summary>
			static float GetFrameTimeMs();
			/// <summary>
			/// フレーム時間の履歴。グラフを描くのに使う。
			/// </summary>
			static const float* GetFrameTimeHistory();
			/// <summary>
			/// フレーム時間の履歴の数。
			/// </summary>
			static int GetFrameTimeHistoryNum();
			/// <summary>
			/// 履歴の中で一番大きいフレーム時間(ミリ秒)。
			/// </summary>
			static float GetFrameTimeMsMax();
			/// <summary>
			/// このフレームのドローコール数。
			/// </summary>
			static int GetDrawCallCount();
			/// <summary>
			/// このフレームに描画されたインスタンスの数。
			/// </summary>
			static int GetDrawInstanceCount();
			/// <summary>
			/// このプロセスが使用しているメインメモリの量(メガバイト)。
			/// </summary>
			static float GetProcessMemoryMB();
			/// <summary>
			/// このプロセスが使用しているビデオメモリの量(メガバイト)。
			/// </summary>
			/// <returns>取得できない場合は-1.0f。</returns>
			static float GetVideoMemoryMB();
			/// <summary>
			/// ビデオメモリの上限(メガバイト)。
			/// </summary>
			/// <returns>取得できない場合は-1.0f。</returns>
			static float GetVideoMemoryBudgetMB();
		};

		/// <summary>
		/// 計測の開始と終了を自動で行うクラス。
		/// </summary>
		/// <remarks>
		/// K2_PROFILE_SCOPE マクロから使われる。
		/// 直接使う必要はない。
		/// </remarks>
		class DebugProfileScope {
		public:
			explicit DebugProfileScope(const char* name)
			{
#ifdef K2_ENABLE_DEBUG_GUI
				DebugProfiler::BeginSection(name);
#else
				(void)name;
#endif
			}
			~DebugProfileScope()
			{
#ifdef K2_ENABLE_DEBUG_GUI
				DebugProfiler::EndSection();
#endif
			}
			DebugProfileScope(const DebugProfileScope&) = delete;
			DebugProfileScope& operator=(const DebugProfileScope&) = delete;
		};
	}
}

// マクロの中で行番号から変数名を作るための小細工。
#define K2_PROFILE_SCOPE_CAT_(a, b) a##b
#define K2_PROFILE_SCOPE_CAT(a, b) K2_PROFILE_SCOPE_CAT_(a, b)

/// <summary>
/// この行から、いまの { } を抜けるまでの時間を計測する。
/// </summary>
/// <remarks>
///     void Player::Update()
///     {
///         K2_PROFILE_SCOPE("Player");
///         ...
///     }
/// </remarks>
#define K2_PROFILE_SCOPE(name)											\
	nsK2EngineLow::nsDbg::DebugProfileScope								\
		K2_PROFILE_SCOPE_CAT(k2ProfileScope_, __LINE__)(name)
