#pragma once

/** ゲームで使うエフェクトの種類（番号はエンジンの EffectEngine に登録するときにも使う） */
enum class EnEffect : int
{
	Hit,			///< 着弾（敵・壁）
	Defeat,			///< 撃破
	Explosion,		///< 爆発弾
	Bullet,			///< 飛んでいる弾（光の筋。止めるまで出続ける）
	Num				///< 種類の数（いちばん最後に置く）
};

/**
 * エフェクトの読み込みと再生
 * ファイル（Assets/effect）は素材の規約で Git に入れないので、別の PC では手でコピーするまでない。ないときは、エフェクトが出ないだけで止めない
 */
namespace nsEffect
{
	/** すべてのエフェクトを読む。Game の Start で 1 回呼ぶ（再生の途中で読むと止まるため） */
	void Load();
	/**
	 * position で再生する。scale は拡大率。終わったら EffectEmitter が自分で消える
	 * 戻り値は再生している EffectEmitter（動かしたり止めたりするときに使う）。読めなかったエフェクトは nullptr
	 */
	EffectEmitter* Play(const EnEffect id, const Vector3& position, const float scale);
}
