#pragma once
#include "param/GameParam.h"

class BattleCharacter;

/** 撃つ係の段階 */
enum class EnShootPhase {
	Wait,			///< 待機
	Aim,			///< 狙う
	Cooldown		///< 休む
};

/** 敵が撃つかどうかを決める係。見つける → 狙う → 撃つ → 休む をくり返す。撃つのは EnemyGun で、この係は決めるだけ。AIController が持つ */
class ShootDecider
{
public:
	/** JSON の shooter の数値を受け取る。借りているだけ（F5 の読み直しが効くように、コピーせずに持つ） */
	void Init(const CharacterParam::Shooter& param);
	/** 段階を進め、このフレームに撃つときに true を返す */
	bool Update(const BattleCharacter& owner, const BattleCharacter& target);
	/** 待機に戻す（湧き直したとき・最初からやり直すとき） */
	void Reset();
	/** どこを狙うか（予測射撃） */
	Vector3 CalcAimPoint(const BattleCharacter& owner, const BattleCharacter& target) const;

private:
	/** 距離が detectRange より近く、間に壁がなければ見つけた */
	bool CanSeeTarget(const BattleCharacter& owner, const BattleCharacter& target) const;
	/** 段階を変え、その段階の時間をタイマーに入れる */
	void ChangePhase(const EnShootPhase phase, const float time);

private:
	const CharacterParam::Shooter* m_param = nullptr;	///< 設定（借りているだけ。Init で受け取る）
	EnShootPhase m_phase = EnShootPhase::Wait;			///< 今の段階
	float m_timer = 0.0f;								///< 今の段階の残り時間（秒）
};
