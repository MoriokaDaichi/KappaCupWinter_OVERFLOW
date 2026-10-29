#include "stdafx.h"
#include "param/GameParam.h"

#include "actor/playerState/PlayerState.h"
#include <fstream>
#include <string>
#include <algorithm>

// キーが足りないときのエラーに「どのグループの」キーかも出す（例: (/swing) key 'maxSpeed' not found）
#define JSON_DIAGNOSTICS 1
#include "thirdParty/nlohmann/json.hpp"

// JSON → 構造体の対応。at() で読むので、キーがなければ例外になる（既定値で黙って動かない）
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CharacterParam::Body, radius, height, chestHeight, tiltLerpRate)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CharacterParam::Move, groundAccel, groundDecel, airControl)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CharacterParam::Jump, speed, maxCount, fastFallAnimSpeed, landAnimMinSpeed)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CharacterParam::WallRun, checkLength, normalYLimit, speed, minSpeed, gravityScale,
	stickSpeed, maxTime, jumpSpeedUp, jumpSpeedOut, reattachTime, tiltAngle)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CharacterParam::Lock, maxDistance, maxAngle, angleWeight, distanceWeight, keepBonus)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CharacterParam::Swing, wireShootSpeed, startLift, pullRate, reelSpeed, minRopeLength,
	reelEase, pumpAccel, faceMinSpeed, maxSpeed, releaseBoost, releaseLift, releaseMinSpeed)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CharacterParam, model, abilities, states, body, move, jump, wallRun, lock, swing)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CameraParam, distance, targetHeight, rotateSpeedX, rotateSpeedY, pitchMin, pitchMax,
	viewAngleNormal, viewAngleMax, viewAngleLerpRate, maxSpeedForFov, collisionRadius, nearClip, farClip, shoulderOffset,
	aimDistance, aimShoulderOffset, aimViewAngleDown, aimRotateScale, aimTransitionTime)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(GravityParam, gravity, fallMultiplier, maxFallSpeed)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AnchorParam, normalScale, lockedScale)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(GrappleWireParam, thicknessScale)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(GunParam::Level, shootInterval, bulletSpeed, bulletScale, pierce, explosionRadius)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(GunParam, range, bulletLifeTime, aimHoldTime, muzzleRight, muzzleForward, levels)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SpeedParam, baseMin, baseMax, bonusMax, groundDecay, idleDecay, wallRunGain, levelUpSpeeds, maxSpeed)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DebugParam::WarpPoint, name, position)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DebugParam, warpPoints)

namespace
{
	GameParam s_param;		///< 読んだ値。Get で返す

	/** 能力の名前が知っているものかを調べる。知らない名前なら、ファイル名つきで例外を投げる */
	void CheckAbilities(const char* path, const CharacterParam& param)
	{
		static const std::array<const char*, 2> KNOWN_ABILITIES = { "Grapple", "Gun" };

		for (const std::string& name : param.abilities)
		{
			if (std::find(KNOWN_ABILITIES.begin(), KNOWN_ABILITIES.end(), name) == KNOWN_ABILITIES.end())
			{
				throw std::runtime_error(std::string(path) + "\nunknown ability: " + name);
			}
		}
	}

	/** 一覧に name があるかを調べる */
	bool Contains(const std::vector<std::string>& list, const char* name)
	{
		return std::find(list.begin(), list.end(), name) != list.end();
	}

	/**
	 * ステートの一覧が正しいか調べる
	 * （知らない名前・Idle がない・Swing なのに Grapple がない）
	 * だめならファイル名付きで例外を投げる
	 */
	void CheckStates(const char* path, const CharacterParam& param)
	{
		// 知らない名前がないか
		for (const std::string& name : param.states)
		{
			EnPlayerState id = EnPlayerState::Num;
			if (!ToPlayerState(name, id))
			{
				throw std::runtime_error(std::string(path) + "\nunknown state: " + name);
			}
		}

		// "Idle" があるか
		if (!Contains(param.states, "Idle"))
		{
			throw std::runtime_error(std::string(path) + "\nstates needs \"Idle\"");
		}

		// "Swing" があるなら "Grapple" もあるか
		if (Contains(param.states, "Swing") && !Contains(param.abilities, "Grapple"))
		{
			throw std::runtime_error(std::string(path) + "\n\"Swing\" needs ability \"Grapple\"");
		}
	}

	/** path の JSON を読んで out に入れる。失敗したら、ファイル名を付けた例外を投げる */
	template <class T>
	void ReadFile(const char* path, T& out)
	{
		std::ifstream file(path);
		if (!file)
		{
			throw std::runtime_error(std::string(path) + "\nfile not found");
		}

		try
		{
			nlohmann::json::parse(file).get_to(out);
		}
		catch (const nlohmann::json::exception& e)
		{
			throw std::runtime_error(std::string(path) + "\n" + e.what());
		}
	}

	/** すべてのファイルを out に読む */
	void ReadAll(GameParam& out)
	{
		ReadFile("Assets/param/chara/player.json", out.player);
		ReadFile("Assets/param/chara/enemy_walker.json", out.enemyWalker);
		ReadFile("Assets/param/camera.json", out.camera);
		ReadFile("Assets/param/gravity.json", out.gravity);
		ReadFile("Assets/param/anchor.json", out.anchor);
		ReadFile("Assets/param/grappleWire.json", out.grappleWire);
		ReadFile("Assets/param/gun.json", out.gun);
		ReadFile("Assets/param/speed.json", out.speed);
		ReadFile("Assets/param/debug.json", out.debug);

		CheckAbilities("Assets/param/chara/player.json", out.player);
		CheckAbilities("Assets/param/chara/enemy_walker.json", out.enemyWalker);

		CheckStates("Assets/param/chara/player.json", out.player);
		CheckStates("Assets/param/chara/enemy_walker.json", out.enemyWalker);

		const size_t speedLevelCount = out.speed.levelUpSpeeds.size() + 1;
		const size_t gunLevelCount = out.gun.levels.size();
		// 銃の Lv ごとの値は、速度の Lv の数だけ必要（足りないと配列の外を読む）
		if (speedLevelCount != gunLevelCount)
		{
			throw std::runtime_error("gun.json: levels count is wrong");
		}
	}
}

namespace nsGameParam
{
	void Load()
	{
		try
		{
			ReadAll(s_param);
		}
		catch (const std::runtime_error& e)
		{
			MessageBoxA(nullptr, e.what(), "GameParam: load failed", MB_OK | MB_ICONERROR);
			exit(EXIT_FAILURE);
		}
	}

	void Reload()
	{
		// 一度別の入れ物に読み、全部そろったときだけ入れ替える（途中で失敗しても前の値が壊れない）
		GameParam param;
		try
		{
			ReadAll(param);
		}
		catch (const std::runtime_error& e)
		{
			MessageBoxA(nullptr, e.what(), "GameParam: reload failed (keep old values)", MB_OK | MB_ICONWARNING);
			return;
		}
		s_param = param;
	}

	const GameParam& Get()
	{
		return s_param;
	}
}
