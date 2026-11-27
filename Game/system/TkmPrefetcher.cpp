#include "stdafx.h"
#include "system/TkmPrefetcher.h"

namespace
{
	/**
	 * 先に読んでおく tkm。ゲームを作るときに時間がかかった順（Release で計測、2026-11-26）
	 * ゲームが ModelRender::Init に渡す文字列と 1 文字も違わないこと（バンクは文字列で探すため）
	 */
	const char* PREFETCH_TKM_PATHS[] =
	{
		"Assets/modelData/player/medea.tkm",			///< 2.7 秒
		"Assets/modelData/enemy/aegis.tkm",				///< 0.6 秒
		"Assets/modelData/player/gun.tkm",				///< 0.4 秒
		"Assets/modelData/enemy/crawler.tkm",			///< 0.1 秒
		"Assets/modelData/enemy/firewall_cannon.tkm",	///< 0.1 秒
		"Assets/modelData/enemy/firewall_base.tkm",		///< 0.1 秒
	};
	constexpr int PREFETCH_TKM_NUM = ARRAYSIZE(PREFETCH_TKM_PATHS);	///< 先に読む tkm の数
}

TkmPrefetcher::~TkmPrefetcher()
{
	if (m_thread.joinable())
	{
		m_thread.join();
	}
}

void TkmPrefetcher::Start()
{
	if (IsAllInBank())
	{
		m_isDone = true;
		return;
	}
	m_thread = std::thread([this]() { Load(); });
}

void TkmPrefetcher::RegistToBank()
{
	if (m_thread.joinable())
	{
		m_thread.join();
	}

	if (m_tkmFiles.empty())
	{
		// 読んでいない（最初から全部バンクにあった）
		return;
	}

	for (int i = 0; i < PREFETCH_TKM_NUM; i++)
	{
		if (g_engine->GetTkmFileFromBank(PREFETCH_TKM_PATHS[i]) != nullptr)
		{
			continue;	// もう誰かが登録していた
		}
		g_engine->RegistTkmFileToBank(PREFETCH_TKM_PATHS[i], m_tkmFiles[i].release());
	}

	m_tkmFiles.clear();
}

void TkmPrefetcher::Load()
{
	m_tkmFiles.reserve(PREFETCH_TKM_NUM);
	for (const char* path : PREFETCH_TKM_PATHS)
	{
		auto tkmFile = std::make_unique<TkmFile>();
		tkmFile->Load(path, false);
		m_tkmFiles.push_back(std::move(tkmFile));
	}
	m_isDone = true;
}

bool TkmPrefetcher::IsAllInBank() const
{
	for (const char* path : PREFETCH_TKM_PATHS)
	{
		if (g_engine->GetTkmFileFromBank(path) == nullptr)
		{
			return false;
		}
	}
	return true;
}
