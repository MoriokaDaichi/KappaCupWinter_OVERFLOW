#pragma once
// TEMPORARY (load-time measurement). Remove after measuring.
// Prints "[LOAD] name detail X ms" to the debugger output only when the scope takes >= 1 ms.
#include <chrono>
#include <cstdio>
#include <Windows.h>

class EngineLoadTimer
{
public:
	explicit EngineLoadTimer(const char* name, const char* detail = "") : m_name(name), m_detail(detail ? detail : ""), m_start(std::chrono::steady_clock::now()) {}
	~EngineLoadTimer()
	{
		const std::chrono::duration<float, std::milli> elapsed = std::chrono::steady_clock::now() - m_start;
		if (elapsed.count() < 1.0f) return;
		char message[512];
		sprintf_s(message, "[LOAD] %s %s %.1f ms\n", m_name, m_detail, elapsed.count());
		OutputDebugStringA(message);
	}
	EngineLoadTimer(const EngineLoadTimer&) = delete;
	EngineLoadTimer& operator=(const EngineLoadTimer&) = delete;
private:
	const char* m_name;
	const char* m_detail;
	std::chrono::steady_clock::time_point m_start;
};
