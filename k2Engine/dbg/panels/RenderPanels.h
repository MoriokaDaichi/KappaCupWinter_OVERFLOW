/*!
 *@brief	ライトとポストエフェクトのデバッグパネル。
 */

#pragma once

namespace nsK2Engine {
	namespace nsDbg {
		/// <summary>
		/// ライトとポストエフェクトのデバッグパネルを登録する。
		/// </summary>
		/// <remarks>
		/// エンジンの初期化時に呼び出される。
		/// ゲーム側から呼び出す必要はない。
		/// </remarks>
		void RegisterRenderPanels();
	}
}
