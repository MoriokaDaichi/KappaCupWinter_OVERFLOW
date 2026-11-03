#include "stdafx.h"
#include "actor/EnemySpawner.h"

#include "actor/BattleCharacter.h"
#include "param/GameParam.h"

void EnemySpawner::Init(const BattleCharacter* player)
{
	m_player = player;

	const std::vector<SpawnParam::Point>& points = nsGameParam::Get().spawn.points;
	m_spawns.resize(points.size());

	for (size_t i = 0; i < points.size(); i++)
	{
		const SpawnParam::Point& point = points[i];
		Spawn& spawn = m_spawns[i];
		spawn.param = nsGameParam::FindEnemy(point.chara);
		spawn.position = Vector3(point.position[0], point.position[1], point.position[2]);
		spawn.respawnTime = point.respawnTime;
		spawn.respawnRadius = point.respawnRadius;
		SpawnEnemy(spawn);
	}
}

void EnemySpawner::Update()
{
	const float dt = g_gameTime->GetFrameDeltaTime();

	for (Spawn& spawn : m_spawns)
	{
		BattleCharacter* enemy = spawn.enemy.Get();
		if (spawn.respawnTime <= 0.0f || enemy == nullptr || enemy->IsActive())
		{
			continue;
		}

		// 区間の外では進めない（遠くで勝手に湧き直さないように）
		const Vector3 toPlayer = m_player->GetPosition() - spawn.position;
		if (toPlayer.LengthSq() > spawn.respawnRadius * spawn.respawnRadius)
		{
			continue;
		}

		spawn.respawnTimer -= dt;
		if (spawn.respawnTimer <= 0.0f)
		{
			enemy->Respawn(spawn.position);
			spawn.respawnTimer = spawn.respawnTime;
		}
	}
}

void EnemySpawner::DeleteAll()
{
	for (Spawn& spawn : m_spawns)
	{
		BattleCharacter* enemy = spawn.enemy.Get();
		if (enemy != nullptr)
		{
			DeleteGO(enemy);
		}
	}
}

void EnemySpawner::SpawnEnemy(Spawn& spawn)
{
	BattleCharacter* enemy = NewGO<BattleCharacter>(0, "enemy");
	enemy->SetStartPosition(spawn.position);
	spawn.controller.SetOwner(enemy);
	spawn.controller.SetTarget(m_player);
	enemy->SetController(&spawn.controller);
	enemy->SetParam(*spawn.param);
	enemy->SetReuseOnDefeat(spawn.respawnTime > 0.0f);

	spawn.enemy.Reset(enemy);
	spawn.respawnTimer = spawn.respawnTime;
}
