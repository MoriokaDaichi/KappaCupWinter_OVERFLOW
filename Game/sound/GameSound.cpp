#include "stdafx.h"
#include "sound/GameSound.h"

#include "param/GameParam.h"
#include <array>

namespace
{
	constexpr int SOUND_NUM = static_cast<int>(EnSound::Num);		///< 音の種類の数

	/** EnSound と同じ順番に並べたファイル名 */
	const char* const SOUND_FILES[] =
	{
		"Assets/sound/bgm.wav",
		"Assets/sound/run.wav",
		"Assets/sound/jump.wav",
		"Assets/sound/shoot.wav",
		"Assets/sound/hit.wav",
		"Assets/sound/defeat.wav",
		"Assets/sound/explosion.wav",
		"Assets/sound/damage.wav",
		"Assets/sound/firewall_break.wav",
		"Assets/sound/grapple_shoot.wav",
		"Assets/sound/grapple_hook.wav",
		"Assets/sound/swing.wav"
	};
	static_assert(ARRAYSIZE(SOUND_FILES) == SOUND_NUM, "SOUND_FILES の数が EnSound と合っていません");

	/** EnSound と同じ順番に並べた、sound.json の音量 */
	float SoundParam::* const SOUND_VOLUMES[] =
	{
		&SoundParam::bgm,
		&SoundParam::run,
		&SoundParam::jump,
		&SoundParam::shoot,
		&SoundParam::hit,
		&SoundParam::defeat,
		&SoundParam::explosion,
		&SoundParam::damage,
		&SoundParam::firewallBreak,
		&SoundParam::grappleShoot,
		&SoundParam::grappleHook,
		&SoundParam::swing
	};
	static_assert(ARRAYSIZE(SOUND_VOLUMES) == SOUND_NUM, "SOUND_VOLUMES の数が EnSound と合っていません");

	std::array<bool, SOUND_NUM> s_isLoaded = {};		///< 読めたか（読めなかった音は鳴らさない。エンジンは読めなかった音を鳴らそうとすると止まるため）
	std::array<SoundSource*, SOUND_NUM> s_loops = {};	///< 鳴らしているくり返す音（自分で消えないので持っておく）

	/** 音を 1 つ作って鳴らす。読めなかった音なら nullptr */
	SoundSource* Start(const EnSound id, const bool isLoop)
	{
		const int number = static_cast<int>(id);
		if (!s_isLoaded[number])
		{
			return nullptr;
		}

		SoundSource* source = NewGO<SoundSource>(0);
		source->Init(number);
		source->SetVolume(nsGameParam::Get().sound.*SOUND_VOLUMES[number]);
		source->Play(isLoop);
		return source;
	}
}

namespace nsSound
{
	void Load()
	{
		for (int i = 0; i < SOUND_NUM; i++)
		{
			g_soundEngine->ResistWaveFileBank(i, SOUND_FILES[i]);
			s_isLoaded[i] = g_soundEngine->GetWaveFileBank().FindWaveFile(i) != nullptr;
		}
	}

	void Play(const EnSound id)
	{
		Start(id, false);
	}

	void SetLoop(const EnSound id, const bool isPlaying, const float volumeRate, const float pitchRate)
	{
		const int number = static_cast<int>(id);
		SoundSource*& source = s_loops[number];
		if (isPlaying && source == nullptr)
		{
			source = Start(id, true);
		}

		if (isPlaying && source != nullptr)
		{
			source->SetVolume(nsGameParam::Get().sound.*SOUND_VOLUMES[number] * volumeRate);
			source->SetFrequencyRatio(pitchRate);
		}
		else if (!isPlaying && source != nullptr)
		{
			source->Stop();
			DeleteGO(source);
			source = nullptr;
		}
	}

	void StopAllLoops()
	{
		for (int i = 0; i < SOUND_NUM; i++)
		{
			SetLoop(static_cast<EnSound>(i), false);
		}
	}
}
