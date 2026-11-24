#pragma once

class BattleCharacter;

/**
 * 敵を倒した瞬間に、画面を一瞬だけ薄く白く光らせる（撃破の手応え）。Game が持つ
 * 白一色だと画面のコントラストが落ちるので、短く・薄くする（LevelUpFlash より控えめ）
 */
class DefeatFlash
{
public:
	/** 見るプレイヤーを受け取り、光の絵を作る（借りているだけ） */
	void Init(const BattleCharacter* player);
	/** 倒した回数が増えたら光り始め、光を少しずつ消す（毎フレーム Game の Update から呼ぶ） */
	void Update();
	/** 光っている間だけ描く（毎フレーム Game の Render から呼ぶ） */
	void Draw(RenderContext& rc);
	/** 消して、数え直す（最初からやり直すとき） */
	void Reset();

private:
	static constexpr float FLASH_TIME = 0.12f;		///< 光が消えるまでの時間（秒）
	static constexpr float FLASH_ALPHA = 0.18f;		///< 光り始めの濃さ

	const BattleCharacter* m_player = nullptr;		///< 見るプレイヤー（借りているだけ）
	SpriteRender m_flash;							///< 画面いっぱいの光
	float m_flashTimer = 0.0f;						///< 光が消えるまでの残り時間（秒）
	int m_lastDefeatCount = 0;						///< 前のフレームで見た、倒した回数
};
