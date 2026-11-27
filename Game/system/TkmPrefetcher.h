#pragma once
#include <thread>
#include <atomic>

/**
 * ゲームで使う重い tkm を、別スレッドで先に読んでおく係
 * タイトルの間に読み、ゲームを作る直前にバンクへ登録する（ゲームを作るときは、バンクから借りるだけになる）
 * 読んでいる間は、メインスレッドで tkm を読まない（ModelRender を作らない）
 * TkmFile::Load がテクスチャのバンクに登録するので、2 つのスレッドから同時に触ると壊れるため
 */
class TkmPrefetcher
{
public:
	/** 読み終わる前に消えるときは、読み終わるまで待つ（スレッドを残したまま消えると落ちるため） */
	~TkmPrefetcher();

public:
	/**
	 * 別スレッドで読み始める。タイトルのモデルを作り終えてから呼ぶ
	 * もう全部バンクにあるとき（2 回目以降のタイトル）は、何も読まずに終わったことにする
	 */
	void Start();
	/** 全部読み終わったか */
	bool IsDone() const { return m_isDone; }
	/**
	 * 読んだ tkm をバンクに登録する。IsDone() が true になってから、メインスレッドで呼ぶ
	 * バンクの中身は std::map で、別スレッドから触ると壊れるため
	 */
	void RegistToBank();

private:
	/** 別スレッドで動く本体。全部読んだら m_isDone を立てる */
	void Load();
	/** 先に読む tkm が、全部もうバンクにあるか（2 回目以降のタイトルでは true） */
	bool IsAllInBank() const;

private:
	std::vector<std::unique_ptr<TkmFile>> m_tkmFiles;	///< 読んだ tkm（PREFETCH_TKM_PATHS と同じ順）。バンクに登録するまでは、ここが持ち主
	std::thread m_thread;								///< 読み込み用のスレッド
	std::atomic<bool> m_isDone = false;					///< 全部読み終わったか（別スレッドが書き、メインスレッドが読む）
};
