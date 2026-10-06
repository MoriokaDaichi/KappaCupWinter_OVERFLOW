/*!
 *@brief	レンダリング設定(ライト、ポストエフェクト)の保存と読み込み。
 */

#pragma once

namespace nsK2Engine {
	namespace nsDbg {
		/// <summary>
		/// レンダリング設定の保存先のファイルパスを取得。
		/// </summary>
		const char* GetRenderSettingsFilePath();

		/// <summary>
		/// 今のレンダリング設定をファイルに保存する。
		/// </summary>
		/// <returns>保存できたらtrue。</returns>
		bool SaveRenderSettings();

		/// <summary>
		/// ファイルからレンダリング設定を読み込んで、今の設定に反映する。
		/// </summary>
		/// <remarks>
		/// ファイルに書かれていない項目は、今の設定のまま残る。
		/// </remarks>
		/// <returns>読み込めたらtrue。ファイルが無い場合はfalse。</returns>
		bool LoadRenderSettings();
	}
}
