/*!
 *@brief	デバッグGUIで調整した値をファイルに保存する。
 */

#pragma once

#include <string>
#include <utility>
#include <vector>

namespace nsK2EngineLow {

	/// <summary>
	/// デバッグGUIで調整した値を、ファイルに保存/復元するためのクラス。
	/// </summary>
	/// <remarks>
	/// ファイルの中身は「キー = 値」が並んだだけの単純なテキストになっている。
	///
	///     # 行の先頭が # ならコメント。
	///     light.ambient           = 0.300 0.300 0.300
	///     postEffect.bloom.enable = 1
	///
	/// メモ帳で開いてそのまま書き換えることもできる。
	///
	/// 【使い方 : 保存】
	///     DebugSettings settings;
	///     settings.SetVector3("light.ambient", ambient);
	///     settings.SetBool("postEffect.bloom.enable", isEnableBloom);
	///     settings.Save("Assets/settings/renderSettings.txt");
	///
	/// 【使い方 : 読み込み】
	///     DebugSettings settings;
	///     if (settings.Load("Assets/settings/renderSettings.txt")) {
	///         ambient = settings.GetVector3("light.ambient", ambient);
	///     }
	///
	/// Get系の関数は、キーが見つからなかった場合に
	/// 第2引数で渡した値をそのまま返す。
	/// そのため「今の値」を第2引数に渡しておけば、
	/// ファイルに書かれていない項目は今の値のまま残る。
	/// </remarks>
	class DebugSettings {
	public:
		/// <summary>
		/// ファイルから読み込む。
		/// </summary>
		/// <param name="filePath">ファイルパス。</param>
		/// <returns>読み込めたらtrue。</returns>
		bool Load(const char* filePath);
		/// <summary>
		/// ファイルに保存する。
		/// </summary>
		/// <remarks>
		/// 保存先のフォルダが無い場合は作成する。
		/// </remarks>
		/// <param name="filePath">ファイルパス。</param>
		/// <returns>保存できたらtrue。</returns>
		bool Save(const char* filePath) const;
		/// <summary>
		/// 中身を空にする。
		/// </summary>
		void Clear()
		{
			m_entries.clear();
		}
		/// <summary>
		/// 中身が空か判定。
		/// </summary>
		bool IsEmpty() const
		{
			return m_entries.empty();
		}

		void SetBool(const char* key, bool value);
		void SetInt(const char* key, int value);
		void SetFloat(const char* key, float value);
		void SetVector3(const char* key, const Vector3& value);

		bool GetBool(const char* key, bool defaultValue) const;
		int GetInt(const char* key, int defaultValue) const;
		float GetFloat(const char* key, float defaultValue) const;
		Vector3 GetVector3(const char* key, const Vector3& defaultValue) const;

	private:
		/// <summary>
		/// キーに対応する値を探す。
		/// </summary>
		/// <returns>見つからなかったらnullptr。</returns>
		const std::string* Find(const char* key) const;
		/// <summary>
		/// キーと値を登録する。すでにあれば上書きする。
		/// </summary>
		void SetString(const char* key, const std::string& value);

		// キーと値の並び。
		// 保存したときのファイルの並び順を、登録した順番のままにしたいので、
		// std::mapではなくvectorを使っている。項目数は多くないので探索も問題にならない。
		std::vector<std::pair<std::string, std::string>> m_entries;
	};
}
