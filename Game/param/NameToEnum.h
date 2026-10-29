#pragma once
#include <string>

/**
 * 名前の表から、name に合う番号を outId に入れる。知らない名前なら false
 * names は enum と同じ順番に名前を並べた配列（JSON に書く名前を enum に直すときに使う）
 */
template <class TEnum, int N>
bool NameToEnum(const char* const (&names)[N], const std::string& name, TEnum& outId)
{
	for (int i = 0; i < N; i++)
	{
		if (name == names[i])
		{
			outId = static_cast<TEnum>(i);
			return true;
		}
	}
	return false;
}
