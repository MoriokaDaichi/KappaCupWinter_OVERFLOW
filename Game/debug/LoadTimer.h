#pragma once
#include <chrono>
#include <Windows.h>

/**
 * 作ってから、置いた { } を抜けるまでの時間を測り、VS の出力ウィンドウに「[LOAD] 名前 〇〇 ms」と出す
 * 読み込みの重さを調べるための道具。測りたい { } の最初に 1 行置くだけで使える
 * 出力は VS から起動したときだけ見える（出力元を「デバッグ」にする）
 * 名前は覚えたポインタを後で使うので、文字列リテラル（"Stage::Init" など）を渡す
 */

class LoadTimer
{
public:
	/** name の区間を測り始める */
	explicit LoadTimer(const char* name) : m_name(name), m_start(std::chrono::steady_clock::now()) {}
	/** 作ってからの時間を出力ウィンドウに出す */
	~LoadTimer()
	{
		const std::chrono::duration<float, std::milli> elapsed = std::chrono::steady_clock::now() - m_start;

		char message[MESSAGE_SIZE];
		sprintf_s(message, "[LOAD] %s %.1f ms\n", m_name, elapsed.count());
		OutputDebugStringA(message);
	}

	/** コピーすると同じ区間が 2 回出るので、コピーできなくする */
	LoadTimer(const LoadTimer&) = delete;
	LoadTimer& operator=(const LoadTimer&) = delete;

private:
	static constexpr int MESSAGE_SIZE = 256;

	const char* m_name;		///< 測る区間の名前
	std::chrono::steady_clock::time_point m_start;		///< 測り始めた時刻
};
