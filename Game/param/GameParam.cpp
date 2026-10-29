#include "stdafx.h"
#include "param/GameParam.h"

#include "actor/characterState/CharacterStateId.h"
#include "actor/characterState/StateTransition.h"
#include "actor/CharacterAnimation.h"
#include <fstream>
#include <string>
#include <algorithm>

// キーが足りないときのエラーに「どのグループの」キーかも出す（例: (/swing) key 'maxSpeed' not found）
#define JSON_DIAGNOSTICS 1
#include "thirdParty/nlohmann/json.hpp"

// JSON → 構造体の対応。at() で読むので、キーがなければ例外になる（既定値で黙って動かない）
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CharacterParam::Body, radius, height, chestHeight, tiltLerpRate)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CharacterParam::Move, groundAccel, groundDecel, airControl)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CharacterParam::Jump, speed, maxCount)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CharacterParam::WallRun, checkLength, normalYLimit, speed, minSpeed, gravityScale,
	stickSpeed, maxTime, jumpSpeedUp, jumpSpeedOut, reattachTime, tiltAngle)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CharacterParam::Lock, maxDistance, maxAngle, angleWeight, distanceWeight, keepBonus)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CharacterParam::Swing, wireShootSpeed, startLift, pullRate, reelSpeed, minRopeLength,
	reelEase, pumpAccel, faceMinSpeed, maxSpeed, releaseBoost, releaseLift, releaseMinSpeed)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CharacterParam::Animation::Clip, anim, file, loop)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CharacterParam::Animation, clips, fastFallSpeed, landMinSpeed)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CharacterParam::Hands, left, right, rightMiddle)
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
	/** 一覧に name があるかを調べる */
	bool Contains(const std::vector<std::string>& list, const char* name)
	{
		return std::find(list.begin(), list.end(), name) != list.end();
	}
}

/** 切り替え表の 1 行を読む。"do" は C++ の予約語なのでマクロが使えず、手で書く（書かなければ、やることなし） */
void from_json(const nlohmann::json& j, CharacterParam::Transition& transition)
{
	j.at("from").get_to(transition.from);
	j.at("when").get_to(transition.when);
	j.at("to").get_to(transition.to);

	transition.actions.clear();
	if (j.contains("do"))
	{
		j.at("do").get_to(transition.actions);
	}
}

/**
 * キャラの JSON を読む
 * wallRun・lock・swing は、使うキャラ（states・abilities に書いたもの）だけ必ず読む
 * animation・hands は見た目なので、書いたキャラだけが持つ（書かなければ、アニメーションなし・手の骨なし）
 * 知らないキー（つづりの間違い）があれば止める（書かなくてもいいキーを打ち間違えると、黙って無視されてしまうため）
 */
void from_json(const nlohmann::json& j, CharacterParam& param)
{
	static const std::array<const char*, 12> KNOWN_KEYS = {
		"model", "abilities", "states", "transitions", "body", "move", "jump",
		"wallRun", "lock", "swing", "animation", "hands"
	};
	for (auto it = j.begin(); it != j.end(); ++it)
	{
		if (std::find(KNOWN_KEYS.begin(), KNOWN_KEYS.end(), it.key()) == KNOWN_KEYS.end())
		{
			throw std::runtime_error("unknown key: " + it.key());
		}
	}

	j.at("model").get_to(param.model);
	j.at("abilities").get_to(param.abilities);
	j.at("states").get_to(param.states);
	j.at("transitions").get_to(param.transitions);
	j.at("body").get_to(param.body);
	j.at("move").get_to(param.move);
	j.at("jump").get_to(param.jump);

	param.hasAnimation = j.contains("animation");
	if (param.hasAnimation)
	{
		j.at("animation").get_to(param.animation);
	}

	param.hasHands = j.contains("hands");
	if (param.hasHands)
	{
		j.at("hands").get_to(param.hands);
	}

	if (Contains(param.states, "WallRun"))
	{
		j.at("wallRun").get_to(param.wallRun);
	}

	if (Contains(param.abilities, "Grapple"))
	{
		j.at("lock").get_to(param.lock);
	}

	if (Contains(param.states, "Swing"))
	{
		j.at("swing").get_to(param.swing);
	}
}

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
			EnCharacterState id = EnCharacterState::Num;
			if (!ToCharacterState(name, id))
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

	/**
	 * 切り替え表が正しいか調べる。だめならファイル名と何行目かを付けて例外を投げる
	 * ・from は states にある State か "Any"、to は states にある State
	 * ・when は 1 つ以上で、知っている条件。その State でしか判定できない条件（Hooked など）は、その State の行だけ
	 * ・WallFound・WallLost は壁走りの値を使うので、states に WallRun が要る
	 * ・do は知っている動作。その State でしかできない動作（WallJump など）は、その State の行だけ
	 */
	void CheckTransitions(const char* path, const CharacterParam& param)
	{
		for (int row = 0; row < static_cast<int>(param.transitions.size()); row++)
		{
			const CharacterParam::Transition& transition = param.transitions[row];
			const std::string where = std::string(path) + "\ntransitions[" + std::to_string(row) + "]: ";

			const bool isFromAny = transition.from == ANY_STATE_NAME;
			EnCharacterState from = EnCharacterState::Num;
			if (!isFromAny && !ToCharacterState(transition.from, from))
			{
				throw std::runtime_error(where + "unknown from: " + transition.from);
			}
			if (!isFromAny && !Contains(param.states, transition.from.c_str()))
			{
				throw std::runtime_error(where + "from \"" + transition.from + "\" is not in states");
			}

			EnCharacterState to = EnCharacterState::Num;
			if (!ToCharacterState(transition.to, to))
			{
				throw std::runtime_error(where + "unknown to: " + transition.to);
			}
			if (!Contains(param.states, transition.to.c_str()))
			{
				throw std::runtime_error(where + "to \"" + transition.to + "\" is not in states");
			}

			if (transition.when.empty())
			{
				throw std::runtime_error(where + "when is empty");
			}
			for (const std::string& name : transition.when)
			{
				EnCondition condition = EnCondition::Num;
				if (!ToCondition(name, condition))
				{
					throw std::runtime_error(where + "unknown condition: " + name);
				}

				const EnCharacterState onlyFrom = CONDITION_ONLY_FROM[static_cast<int>(condition)];
				if (onlyFrom != EnCharacterState::Num && (isFromAny || from != onlyFrom))
				{
					throw std::runtime_error(where + "condition \"" + name + "\" can be used only from " + STATE_NAMES[static_cast<int>(onlyFrom)]);
				}

				const bool usesWallRun = condition == EnCondition::WallFound || condition == EnCondition::WallLost;
				if (usesWallRun && !Contains(param.states, "WallRun"))
				{
					throw std::runtime_error(where + "condition \"" + name + "\" needs state \"WallRun\"");
				}
			}

			for (const std::string& name : transition.actions)
			{
				EnTransitionAction action = EnTransitionAction::Num;
				if (!ToTransitionAction(name, action))
				{
					throw std::runtime_error(where + "unknown action: " + name);
				}

				const EnCharacterState onlyFrom = ACTION_ONLY_FROM[static_cast<int>(action)];
				if (onlyFrom != EnCharacterState::Num && (isFromAny || from != onlyFrom))
				{
					throw std::runtime_error(where + "action \"" + name + "\" can be used only from " + STATE_NAMES[static_cast<int>(onlyFrom)]);
				}
			}
		}
	}

	/** アニメーションの一覧が正しいか調べる（知らない名前・同じ名前が 2 回・足りない名前）。だめならファイル名付きで例外を投げる */
	void CheckAnimation(const char* path, const CharacterParam& param)
	{
		if (!param.hasAnimation)
		{
			return;
		}

		std::array<bool, static_cast<int>(EnCharacterAnim::Num)> isWritten = {};
		for (const CharacterParam::Animation::Clip& clip : param.animation.clips)
		{
			EnCharacterAnim anim = EnCharacterAnim::Num;
			if (!ToCharacterAnim(clip.anim, anim))
			{
				throw std::runtime_error(std::string(path) + "\nunknown anim: " + clip.anim);
			}

			bool& written = isWritten[static_cast<int>(anim)];
			if (written)
			{
				throw std::runtime_error(std::string(path) + "\nanim \"" + clip.anim + "\" is written twice");
			}
			written = true;
		}

		for (int i = 0; i < static_cast<int>(EnCharacterAnim::Num); i++)
		{
			if (!isWritten[i])
			{
				throw std::runtime_error(std::string(path) + "\nanimation needs anim \"" + ANIM_NAMES[i] + "\"");
			}
		}
	}

	/** キャラ 1 人分の中身を調べる（能力 → State → 切り替え表 → アニメーションの順。後のものほど前のものを使う） */
	void CheckCharacter(const char* path, const CharacterParam& param)
	{
		CheckAbilities(path, param);
		CheckStates(path, param);
		CheckTransitions(path, param);
		CheckAnimation(path, param);
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
		// from_json の中で投げた、知らないキーのエラー
		catch (const std::runtime_error& e)
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

		CheckCharacter("Assets/param/chara/player.json", out.player);
		CheckCharacter("Assets/param/chara/enemy_walker.json", out.enemyWalker);

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
