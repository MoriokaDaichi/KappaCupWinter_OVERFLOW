#pragma once
#include "controller/AIController.h"
#include <vector>

class BattleCharacter;
class BulletSystem;
struct CharacterParam;

/**
 * spawn.json に書いた場所に敵を出す。respawnTime を書いた場所は、倒されたら湧き直す（敵が湧き直す区間）。Game が持つ
 * 敵を作るときはモデルの準備でゲームが止まるので、作るのは Init の 1 回だけにする。湧き直す敵は、倒されたら隠しておいて使い回す
 * 場所は Init のときに写すので、JSON を読み直しても変わらない（ゲームの途中で敵の数が変わると、持っている枠とずれるため）
 */
class EnemySpawner
{
public:
	/** spawn.json の場所すべてに敵を出す。player は敵が追いかける相手で、距離も測る（借りているだけ） */
	void Init(const BattleCharacter* player, BulletSystem* bulletSystem);
	/** 毎フレーム Game の Update から呼ぶ。倒された敵の湧き直しの時間を進める */
	void Update();
	/** 生きている敵を全部消す（Game が消えるときに呼ぶ） */
	void DeleteAll();

private:
	/** 敵を出す場所 1 つ分 */
	struct Spawn
	{
		const CharacterParam* param = nullptr;		///< 敵の設定（nsGameParam が持っているものを借りる）
		Vector3 position;							///< 足元の位置
		float respawnTime = 0.0f;					///< 倒されてから湧き直すまでの時間（秒）。0 なら湧き直さない
		float respawnRadius = 0.0f;					///< プレイヤーがこの距離以内にいる間だけ、時間が進む
		float respawnTimer = 0.0f;					///< 湧き直すまでの残り時間
		AIController controller;					///< この敵のコマンドを作る（敵が借りるので、m_spawns の大きさは Init の後に変えない）
		GOHandle<BattleCharacter> enemy;			///< この場所の敵（湧き直す敵は、倒されても消えずに隠れている）
		bool canShoot = false;						///< 仮: 弾を撃つ敵か（11/6 で置き換える）
		float shootTimer = 0.0f;					///< 仮: 次に撃つまでの残り時間（11/6 で置き換える）
	};

	/** spawn の場所に敵を 1 体出す */
	void SpawnEnemy(Spawn& spawn);
	/** 仮: enemy からプレイヤーへ弾を 1 発撃つ（11/6 で置き換える） */
	void Shoot(const BattleCharacter& enemy);

private:
	static constexpr float TEMP_SHOOT_INTERVAL = 1.0f;	///< 仮: 弾を撃つ間隔（秒。予測射撃のタスクで置き換える）
	static constexpr float TEMP_SHOOT_RANGE = 5000.0f;	///< 仮: プレイヤーがこの距離より遠いと撃たない（プレイヤー検知のタスクで置き換える）

	std::vector<Spawn> m_spawns;					///< 敵を出す場所の一覧
	const BattleCharacter* m_player = nullptr;		///< プレイヤー（借りているだけ）
	BulletSystem* m_bulletSystem = nullptr;	///< 弾を出す先（借りているだけ）
};
