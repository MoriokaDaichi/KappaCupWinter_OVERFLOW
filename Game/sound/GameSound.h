#pragma once

/** ゲームで使う音の種類（番号はエンジンの WaveFileBank に登録するときにも使う） */
enum class EnSound : int
{
	Bgm,			///< BGM（くり返す）
	Run,			///< 走っている間の音（くり返す）
	Jump,			///< ジャンプ
	Shoot,			///< 射撃
	Hit,			///< 着弾（プレイヤーの弾が敵に当たった）
	Defeat,			///< 撃破
	Explosion,		///< 爆発弾（敵を巻き込んだ）
	Damage,			///< 被弾（プレイヤー）
	FirewallBreak,	///< 巨大ファイアウォールを突き破る
	GrappleShoot,	///< グラップルのワイヤーを撃つ
	GrappleHook,	///< グラップルがアンカーに刺さる
	Swing,			///< スイング中の「シュィーン」（くり返す）
	Num				///< 種類の数（いちばん最後に置く）
};

/**
 * 音の読み込みと再生。音量は Assets/param/sound.json（F5 で読み直すと、次に鳴らす音から変わる）
 * 距離で音量が変わらない 2D の音だけを使う。遠くの音（敵の射撃など）は鳴らさない
 */
namespace nsSound
{
	/** すべての音を読む。Game の Start で 1 回呼ぶ。読めなかった音は鳴らさない（止めない） */
	void Load();
	/** 1 回鳴らす。鳴り終わったら SoundSource が自分で消える */
	void Play(const EnSound id);
	/**
	 * くり返す音を鳴らすか止めるかを決める。同じ音は 1 つしか鳴らさないので、毎フレーム呼んでよい
	 * @param isPlaying		true で鳴らす（鳴っていれば何もしない）、false で止める
	 * @param volumeRate	sound.json の音量に掛ける割合（速さで大きさを変えるときに使う）
	 * @param pitchRate		音の高さの割合（1 でそのまま、2 で 1 オクターブ上。2 まで）
	 */
	void SetLoop(const EnSound id, const bool isPlaying, const float volumeRate = 1.0f, const float pitchRate = 1.0f);
	/** くり返す音を全部止めて消す。Game が消えるときに呼ぶ（くり返す音は自分で消えないため） */
	void StopAllLoops();
}
