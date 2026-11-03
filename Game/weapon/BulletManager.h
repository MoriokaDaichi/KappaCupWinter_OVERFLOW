#pragma once
#include "param/GameParam.h"
#include <vector>

class BattleCharacter;

/** 撃った弾をまとめて動かし、まとめて描く（１発ごとにInitすると重いため）。敵に当たったらダメージを与える。Game が NewGO する */
class BulletManager : public IGameObject
{
public:
	bool Start() override;
	void Update() override;
	void Render(RenderContext& rc) override;

public:
	/** 空いている枠を探して弾を出す。空きがなければ撃たない。shooter は撃った本人（敵を倒すと基礎速度が上がる） */
	void Fire(const Vector3& position, const Vector3& velocity, const GunParam::Level& levelParam, BattleCharacter* shooter);
	/** target の方へ曲がっていく弾を出す */
	void FireHoming(const Vector3& position, const Vector3& velocity, const GunParam::Level& levelParam, BattleCharacter* shooter,
		const GOHandle<BattleCharacter>& target);

private:
	/** 弾を消して空きに戻す（描かれないように大きさ 0 にする） */
	void Deactivate(const int index);

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
		GOHandle<BattleCharacter> shooter;		///< 撃った本人
		GOHandle<BattleCharacter> target;		///< ホーミングする相手（空か、相手が倒されたら、まっすぐ飛ぶ）
		std::vector<uint64_t> hitEnemyIds;		///< ダメージを与えた敵のインスタンス ID（貫通弾が同じ敵に何度も当たらないように）
	};

	/** 弾の向きを target の方へ曲げる。相手が倒されていたら何もしない */
	void UpdateHoming(Bullet& bullet, const float dt) const;
	/** 今いる敵を m_enemies に集める（弾の数だけ探すと重いので、1 フレームに 1 回） */
	void CollectEnemies();
	/** 弾の今の位置から end までに当たる敵全部にダメージを与える。弾が止まる（貫通しない弾が当たった）なら true */
	bool HitEnemies(Bullet& bullet, const Vector3& end);
	/** 弾の今の位置から end までで、最初に当たる敵を返す。いなければ nullptr */
	BattleCharacter* FindFirstHit(const Bullet& bullet, const Vector3& end, Vector3& hitPoint) const;
	/** この弾でもうダメージを与えた敵か */
	bool HasHit(const Bullet& bullet, const BattleCharacter& enemy) const;
	/** 敵にダメージを与える。倒したら、撃った本人の基礎速度を上げる（体は押し出さない） */
	void DamageEnemy(Bullet& bullet, BattleCharacter& enemy);
	/** 弾が止まる所（敵・壁）で、爆発弾なら爆発し、それ以外は着弾のエフェクトを出す */
	void OnBulletImpact(Bullet& bullet, const Vector3& position);
	/** center を中心に爆発する（半径内の、この弾でまだダメージを与えていない敵にダメージ） */
	void Explode(Bullet& bullet, const Vector3& center);

	static constexpr int MAX_BULLET = 128;					///< 同時に飛べる弾の数（連射 0.08 秒 × 寿命 2 秒 ≒ 25 発に余裕を持たせる）
	static constexpr float MODEL_RADIUS = 50.0f;			///< 弾のモデル（debug_anchor.tkm）の大きさ 1 のときの半径
	std::array<Bullet, MAX_BULLET> m_bullets;				///< 全部の弾。isActive が false の枠は空き
	std::vector<BattleCharacter*> m_enemies;				///< このフレームの敵（Update の中でだけ使う）
	ModelRender m_modelRender;								///< 弾の見た目（全部の弾でこれ1つ）
	const GunParam& m_param = nsGameParam::Get().gun;		///< 調整用の値（Assets/param/gun.json）
	const EffectParam& m_effectParam = nsGameParam::Get().effect;	///< エフェクトの拡大率（Assets/param/effect.json）
};
