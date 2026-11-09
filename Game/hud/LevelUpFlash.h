#pragma once

class BattleCharacter;

/** 速度 Lv が上がった瞬間に、画面をシアンで一瞬光らせる（Lv アップの演出の最小版）。Game が持つ */
class LevelUpFlash
{
public:
	/** 見るプレイヤーを受け取り、光の絵を作る（借りているだけ） */
	void Init(const BattleCharacter* player);
	/** Lv が上がったら光り始め、光を少しずつ消す（毎フレーム Game の Update から呼ぶ） */
	void Update();
	/** 光っている間だけ描く（毎フレーム Game の Render から呼ぶ） */
	void Draw(RenderContext& rc);

private:
	static constexpr float FLASH_TIME = 0.3f;		///< 光が消えるまでの時間（秒）。巨大ファイアウォールの突破より短くする
	static constexpr float FLASH_ALPHA = 0.4f;		///< 光り始めの濃さ（画面が見えなくならない程度）

	const BattleCharacter* m_player = nullptr;		///< 見るプレイヤー（借りているだけ）
	SpriteRender m_flash;							///< 画面いっぱいの光
	float m_flashTimer = 0.0f;						///< 光が消えるまでの残り時間（秒）
	int m_lastLevel = 0;							///< 前のフレームの Lv（上がった瞬間を知るため）
};
