#include "stdafx.h"
#include "param/GameParam.h"

#include <fstream>
#include <string>

// キーが足りないときのエラーに「どのグループの」キーかも出す（例: (/swing) key 'maxSpeed' not found）
#define JSON_DIAGNOSTICS 1
#include "thirdParty/nlohmann/json.hpp"

// JSON → 構造体の対応。at() で読むので、キーがなければ例外になる（既定値で黙って動かない）
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PlayerParam::Body, chestHeight)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PlayerParam::Move, groundAccel, groundDecel, airControl)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PlayerParam::Jump, speed, maxCount)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PlayerParam::WallRun, checkLength, normalYLimit, speed, minSpeed, gravityScale,
	stickSpeed, maxTime, jumpSpeedUp, jumpSpeedOut, reattachTime)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PlayerParam::Lock, maxDistance, maxAngle, angleWeight, distanceWeight, keepBonus)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PlayerParam::Swing, wireShootSpeed, startLift, pullRate, reelSpeed, minRopeLength,
	reelEase, pumpAccel, faceMinSpeed, maxSpeed, releaseBoost, releaseLift, releaseMinSpeed)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PlayerParam, body, move, jump, wallRun, lock, swing)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CameraParam, distance, targetHeight, rotateSpeedX, rotateSpeedY, pitchMin, pitchMax,
	viewAngleNormal, viewAngleMax, viewAngleLerpRate, maxSpeedForFov, collisionRadius, nearClip, farClip, shoulderOffset,
	aimDistance, aimShoulderOffset, aimViewAngleDown, aimRotateScale, aimTransitionTime)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(GravityParam, gravity, fallMultiplier, maxFallSpeed)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AnchorParam, normalScale, lockedScale)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(GrappleWireParam, thicknessScale)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(GunParam, shootInterval, range, bulletSpeed, bulletLifeTime, bulletScale, aimHoldTime,
	muzzleRight, muzzleForward)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SpeedParam, baseMin, baseMax, bonusMax, groundDecay, idleDecay, wallRunGain, levelUpSpeeds, maxSpeed)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DebugParam::WarpPoint, name, position)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DebugParam, warpPoints)

namespace
{
	GameParam s_param;		///< 読んだ値。Get で返す

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
		ReadFile("Assets/param/player.json", out.player);
		ReadFile("Assets/param/camera.json", out.camera);
		ReadFile("Assets/param/gravity.json", out.gravity);
		ReadFile("Assets/param/anchor.json", out.anchor);
		ReadFile("Assets/param/grappleWire.json", out.grappleWire);
		ReadFile("Assets/param/gun.json", out.gun);
		ReadFile("Assets/param/speed.json", out.speed);
		ReadFile("Assets/param/debug.json", out.debug);
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
