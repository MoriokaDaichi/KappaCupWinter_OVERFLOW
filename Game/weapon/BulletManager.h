#pragma once
#include "param/GameParam.h"
#include "effect/EffectManager.h"
#include <vector>

class BattleCharacter;

/** 弾 1 発分の設定（撃つ側が作って Fire に渡す） */
struct BulletShot
{
	EnEffect tracer = EnEffect::Bullet;			///< 飛んでいる間のエフェクト（色の違いはここで出す）
	float lifeTime = 0.0f;						///< 消えるまでの時間（秒）
	float scale = 0.0f;							///< 大きさ
	float damage = 0.0f;						///< ダメージ（爆発も同じ量）
	bool pierce = false;						///< 弾が貫通するかどうか
	float explosionRadius = 0.0f;				///< 爆発の半径（0 なら爆発しない）
};

/** 撃った弾をまとめて動かし、まとめて描く（１発ごとにInitすると重いため）。Init で渡した名前の相手に当たったらダメージを与える。Game が NewGO する */
class BulletManager : public IGameObject
{
public:
	bool Start() override;
	void Update() override;
	void Render(RenderContext& rc) override;

public:
	/** 当てる相手の名前を渡す（NewGO の直後に呼ぶ）。targetName は、相手を NewGO したときの名前 */
	void Init(const char* targetName) { m_targetName = targetName; }
	/** 空いている枠を探して弾を出す。空きがなければ撃たない。shooter は撃った本人（相手を倒すと基礎速度が上がる） */
	void Fire(const Vector3& position, const Vector3& velocity, const BulletShot& shot, BattleCharacter* shooter);
	/** target の方へ曲がっていく弾を出す */
	void FireHoming(const Vector3& position, const Vector3& velocity, const BulletShot& shot, BattleCharacter* shooter,
		const GOHandle<BattleCharacter>& target);
	/** 少し飛んだ後に、targets の数に分裂してホーミングする弾を出す */
	void FireSplit(const Vector3& position, const Vector3& velocity, const BulletShot& shot, BattleCharacter* shooter,
		const std::vector<GOHandle<BattleCharacter>>& targets);

private:
	/** 弾の見た目を今の位置と向きに合わせる */
	void UpdateBulletView(const int index);
	/** 弾を消して空きに戻す（描かれないように大きさ 0 にする） */
	void Deactivate(const int index);
	/** 時間が来た弾を分裂させて消す。消したら true */
	void UpdateSplit(const int index, const float dt);

private:
	/** 弾1発分 */
	struct Bullet
	{
		bool isActive = false;		///< 飛んでいるか（false なら空き）
		Vector3 position;			///< 今の位置
		Vector3 velocity;			///< 速度
		float lifeTimer = 0.0f;		///< 消えるまでの残り時間
		float scale = 0.0f;			///< 大きさ
		bool pierce = false;		///< 弾が貫通するかどうか
		float explosionRadius = 0.0f;	///< 爆発の半径（0 なら爆発しない）
		float damage = 0.0f;		///< ダメージ
		BulletShot splitShot;										///< 分裂した後の弾の設定
		float splitTimer = 0.0f;									///< 分裂するまでの残り時間
		std::vector<GOHandle<BattleCharacter>> splitTargets;		///< 分裂して向かう相手（空なら分裂しない）
		GOHandle<BattleCharacter> shooter;		///< 撃った本人
		GOHandle<BattleCharacter> target;		///< ホーミングする相手（空か、相手が倒されたら、まっすぐ飛ぶ）
		GOHandle<EffectEmitter> tracer;			///< 弾の見た目の光の筋（エフェクトのファイルがないときは空で、代わりに球を描く）
		std::vector<uint64_t> hitTargetIds;		///< ダメージを与えた相手のインスタンス ID（貫通弾が同じ相手に何度も当たらないように）
	};

	/** 弾の向きを target の方へ曲げる。相手が倒されていたら何もしない */
	void UpdateHoming(Bullet& bullet, const float dt) const;
	/** 今いる相手を m_hitTargets に集める（弾の数だけ探すと重いので、1 フレームに 1 回） */
	void CollectTargets();
	/** 弾の今の位置から end までに当たる相手全部にダメージを与える。弾が止まる（貫通しない弾が当たった）なら true */
	bool HitTargets(Bullet& bullet, const Vector3& end);
	/** 弾の今の位置から end までで、最初に当たる相手を返す。いなければ nullptr */
	BattleCharacter* FindFirstHit(const Bullet& bullet, const Vector3& end, Vector3& hitPoint) const;
	/** この弾でもうダメージを与えた相手か */
	bool HasHit(const Bullet& bullet, const BattleCharacter& hitTarget) const;
	/** 相手にダメージを与える。倒したら、撃った本人の基礎速度を上げる（体は押し出さない） */
	void DamageTarget(Bullet& bullet, BattleCharacter& hitTarget);
	/** 弾が止まる所（相手・壁）で、爆発弾なら爆発し、それ以外は着弾のエフェクトを出す */
	void OnBulletImpact(Bullet& bullet, const Vector3& position);
	/** center を中心に爆発する（半径内の、この弾でまだダメージを与えていない相手にダメージ） */
	void Explode(Bullet& bullet, const Vector3& center);

	static constexpr int MAX_BULLET = 128;					///< 同時に飛べる弾の数（連射 0.08 秒 × 寿命 2 秒 ≒ 25 発に余裕を持たせる）
	static constexpr float MODEL_RADIUS = 50.0f;			///< 弾のモデル（debug_anchor.tkm）の大きさ 1 のときの半径
	std::array<Bullet, MAX_BULLET> m_bullets;				///< 全部の弾。isActive が false の枠は空き
	std::vector<BattleCharacter*> m_hitTargets;				///< このフレームの相手（Update の中でだけ使う）
	ModelRender m_modelRender;								///< エフェクトのファイルがないときの弾の見た目（全部の弾でこれ1つ）
	const GunParam& m_param = nsGameParam::Get().gun;		///< 調整用の値（Assets/param/gun.json）
	const EffectParam& m_effectParam = nsGameParam::Get().effect;	///< エフェクトの拡大率（Assets/param/effect.json）
	const char* m_targetName = nullptr;						///< 当てる相手の名前（"enemy" か "player"）
};
