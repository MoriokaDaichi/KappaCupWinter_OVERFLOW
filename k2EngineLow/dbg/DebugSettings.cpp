/*!
 *@brief	デバッグGUIで調整した値をファイルに保存する。
 */

#include "k2EngineLowPreCompile.h"
#include "DebugSettings.h"

namespace nsK2EngineLow {

	namespace {
		/// <summary>
		/// 文字列の前後の空白を取り除く。
		/// </summary>
		std::string Trim(const std::string& text)
		{
			const char* whiteSpaces = " \t\r\n";
			size_t begin = text.find_first_not_of(whiteSpaces);
			if (begin == std::string::npos) {
				return std::string();
			}
			size_t end = text.find_last_not_of(whiteSpaces);
			return text.substr(begin, end - begin + 1);
		}

		/// <summary>
		/// 保存先のフォルダが無ければ作成する。
		/// </summary>
		void CreateParentDirectory(const char* filePath)
		{
			std::string path = filePath;
			// 区切り文字を円マークに統一する。
			for (auto& c : path) {
				if (c == '/') {
					c = '\\';
				}
			}
			// 手前から順にフォルダを作っていく。
			size_t separator = path.find('\\');
			while (separator != std::string::npos) {
				std::string directory = path.substr(0, separator);
				if (directory.empty() == false) {
					CreateDirectoryA(directory.c_str(), nullptr);
				}
				separator = path.find('\\', separator + 1);
			}
		}
	}

	bool DebugSettings::Load(const char* filePath)
	{
		FILE* file = nullptr;
		if (fopen_s(&file, filePath, "r") != 0 || file == nullptr) {
			return false;
		}
		m_entries.clear();

		char line[1024];
		while (fgets(line, sizeof(line), file) != nullptr) {
			std::string text = Trim(line);
			if (text.empty() || text[0] == '#') {
				// 空行とコメント行は読み飛ばす。
				continue;
			}
			size_t equal = text.find('=');
			if (equal == std::string::npos) {
				// 「キー = 値」の形になっていない行は無視する。
				continue;
			}
			std::string key = Trim(text.substr(0, equal));
			std::string value = Trim(text.substr(equal + 1));
			if (key.empty()) {
				continue;
			}
			SetString(key.c_str(), value);
		}
		fclose(file);
		return true;
	}

	bool DebugSettings::Save(const char* filePath) const
	{
		CreateParentDirectory(filePath);

		FILE* file = nullptr;
		if (fopen_s(&file, filePath, "w") != 0 || file == nullptr) {
			return false;
		}
		fprintf(file, "# k2Engine debug settings\n");
		fprintf(file, "# Saved from the debug GUI. You can edit this file by hand.\n");
		fprintf(file, "\n");

		// キーの長さを揃えて、読みやすく書き出す。
		size_t keyWidth = 0;
		for (const auto& entry : m_entries) {
			keyWidth = (std::max)(keyWidth, entry.first.length());
		}
		for (const auto& entry : m_entries) {
			fprintf(
				file,
				"%-*s = %s\n",
				(int)keyWidth,
				entry.first.c_str(),
				entry.second.c_str()
			);
		}
		fclose(file);
		return true;
	}

	const std::string* DebugSettings::Find(const char* key) const
	{
		for (const auto& entry : m_entries) {
			if (entry.first == key) {
				return &entry.second;
			}
		}
		return nullptr;
	}

	void DebugSettings::SetString(const char* key, const std::string& value)
	{
		for (auto& entry : m_entries) {
			if (entry.first == key) {
				entry.second = value;
				return;
			}
		}
		m_entries.push_back(std::make_pair(std::string(key), value));
	}

	void DebugSettings::SetBool(const char* key, bool value)
	{
		SetString(key, value ? "1" : "0");
	}

	void DebugSettings::SetInt(const char* key, int value)
	{
		char buffer[64];
		sprintf_s(buffer, "%d", value);
		SetString(key, buffer);
	}

	void DebugSettings::SetFloat(const char* key, float value)
	{
		char buffer[64];
		sprintf_s(buffer, "%.4f", value);
		SetString(key, buffer);
	}

	void DebugSettings::SetVector3(const char* key, const Vector3& value)
	{
		char buffer[128];
		sprintf_s(buffer, "%.4f %.4f %.4f", value.x, value.y, value.z);
		SetString(key, buffer);
	}

	bool DebugSettings::GetBool(const char* key, bool defaultValue) const
	{
		return GetInt(key, defaultValue ? 1 : 0) != 0;
	}

	int DebugSettings::GetInt(const char* key, int defaultValue) const
	{
		const std::string* value = Find(key);
		if (value == nullptr) {
			return defaultValue;
		}
		int result = defaultValue;
		if (sscanf_s(value->c_str(), "%d", &result) != 1) {
			return defaultValue;
		}
		return result;
	}

	float DebugSettings::GetFloat(const char* key, float defaultValue) const
	{
		const std::string* value = Find(key);
		if (value == nullptr) {
			return defaultValue;
		}
		float result = defaultValue;
		if (sscanf_s(value->c_str(), "%f", &result) != 1) {
			return defaultValue;
		}
		return result;
	}

	Vector3 DebugSettings::GetVector3(const char* key, const Vector3& defaultValue) const
	{
		const std::string* value = Find(key);
		if (value == nullptr) {
			return defaultValue;
		}
		Vector3 result = defaultValue;
		if (sscanf_s(value->c_str(), "%f %f %f", &result.x, &result.y, &result.z) != 3) {
			return defaultValue;
		}
		return result;
	}
}
