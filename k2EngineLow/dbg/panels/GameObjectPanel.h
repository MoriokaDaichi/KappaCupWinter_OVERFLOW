/*!
 *@brief	ゲームオブジェクトの一覧を表示するデバッグパネル。
 */

#pragma once

namespace nsK2EngineLow {
	namespace nsDbg {
		/// <summary>
		/// ゲームオブジェクトのデバッグパネルを登録する。
		/// </summary>
		/// <remarks>
		/// エンジンの初期化時に呼び出される。
		/// ゲーム側から呼び出す必要はない。
		/// </remarks>
		void RegisterGameObjectPanel();
	}
}
