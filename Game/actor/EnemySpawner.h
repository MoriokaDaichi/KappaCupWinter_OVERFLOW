#pragma once
#include "controller/AIController.h"
#include <vector>

class BattleCharacter;
struct CharacterParam;

/**
 * spawn.json に書いた場所に敵を出す。respawnTime を書いた場所は、倒されたら湧き直す（敵が湧き直す区間）。 
 * 湧き直す敵は、倒されたら隠しておいて使い回す。
 * 敵を作るときはモデルの準備でゲームが止まるので、作るのはゲームの読み込みの間だけにする（Init で場所を写し、SpawnNext で数体ずつ作る）。
 * 場所は Init のときに写すので、JSON を読み直しても変わらない（ゲームの途中で敵の数が変わると、持っている枠とずれるため）
 */
class EnemySpawner
{
public:
	/** spawn.json の場所を写す（敵はまだ出さない。SpawnNext で出す）。player は敵が追いかける相手で、距離も測る（借りているだけ） */
	void Init(const BattleCharacter* player);
	/** 毎フレーム Game の Update から呼ぶ。倒された敵の湧き直しの時間を進める */
	void Update();
	/** 生きている敵を全部消す（Game が消えるときに呼ぶ） */
	void DeleteAll();
	/**
	 * 全部の敵を最初の場所に戻す（最初からやり直すとき）
	 * 湧き直さない敵は倒されると消えているので、作り直す（そのときはモデルの準備で少し止まる）
	 */
	void Reset();
	/**
	 * まだ出していない敵を、1 回に SPAWN_NUM_PER_CALL 体まで出す。全部出し終わったら true
	 * Game::Start から毎フレーム呼ぶ（敵の Start は重いので、1 フレームに集めないため）
	 */
	bool SpawnNext();

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
	};

	/** spawn の場所に敵を 1 体出す */
	void SpawnEnemy(Spawn& spawn);

private:
	static constexpr int SPAWN_NUM_PER_CALL = 1;	///< 1 回に出す敵の数（敵の Start は 1 体 0.03〜0.17 秒かかるので、1 フレームに 1 体ずつにしてダイブの絵を止めない）

	int m_nextSpawnIndex = 0;						///< 次に出す m_spawns の番号
	std::vector<Spawn> m_spawns;					///< 敵を出す場所の一覧
	const BattleCharacter* m_player = nullptr;		///< プレイヤー（借りているだけ）
};
