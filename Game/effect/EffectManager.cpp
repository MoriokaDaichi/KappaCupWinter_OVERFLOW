#include "stdafx.h"
#include "effect/EffectManager.h"

#include <array>

namespace
{
	constexpr int EFFECT_NUM = static_cast<int>(EnEffect::Num);		///< エフェクトの種類の数

	/** EnEffect と同じ順番に並べたファイル名 */
	const char16_t* const EFFECT_FILES[] =
	{
		u"Assets/effect/hit.efk",
		u"Assets/effect/defeat.efk",
		u"Assets/effect/explosion.efk",
		u"Assets/effect/bullet.efk",
		u"Assets/effect/enemy_bullet.efk"
	};
	static_assert(ARRAYSIZE(EFFECT_FILES) == EFFECT_NUM, "EFFECT_FILES の数が EnEffect と合っていません");

	std::array<bool, EFFECT_NUM> s_isLoaded = {};		///< 読めたか（読めなかったエフェクトは再生しない）
}

namespace nsEffect
{
	void Load()
	{
		EffectEngine* engine = EffectEngine::GetInstance();
		for (int i = 0; i < EFFECT_NUM; i++)
		{
			engine->ResistEffect(i, EFFECT_FILES[i]);
			s_isLoaded[i] = engine->LoadEffect(i).Get() != nullptr;
		}
	}

	EffectEmitter* Play(const EnEffect id, const Vector3& position, const float scale)
	{
		const int number = static_cast<int>(id);
		if (!s_isLoaded[number])
		{
			return nullptr;
		}

		EffectEmitter* emitter = NewGO<EffectEmitter>(0);
		emitter->Init(number);
		emitter->SetPosition(position);
		emitter->SetScale(Vector3(scale, scale, scale));
		emitter->Play();
		return emitter;
	}
}
