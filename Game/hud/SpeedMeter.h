#pragma once
#include "hud/HudShape.h"
#include "hud/HudText.h"

class BattleCharacter;

/**
 * 画面右下の 4 分円のスピードメーター。針は今の Lv の中の進み具合で、振り切ると Lv が上がって根元に戻る
 * Lv5 で振り切ると OVERDRIVE になり、針が張り付いて弧がシアンで光る。Hud が持つ
 */
class SpeedMeter
{
public:
	/** 見るプレイヤーを受け取る（借りているだけ） */
	void Init(const BattleCharacter* player);
	/** 毎フレーム Hud の Draw から呼ぶ */
	void Draw();

private:
	static constexpr float METER_SIZE = 420.0f;			///< メーターの大きさ（4 分円の半径。ピクセル）
	static constexpr float SCREEN_MARGIN = 24.0f;		///< 画面の端からの空き（ピクセル）
	static constexpr float LEVEL_X_RATE = 0.42f;		///< Lv の数字の位置（右下の角から左へ、メーターの大きさに対する割合）
	static constexpr float LEVEL_Y_RATE = 0.26f;		///< Lv の数字の位置（右下の角から上へ）
	static constexpr float LEVEL_HEIGHT = 120.0f;		///< Lv の数字の高さ（ピクセル）
	static constexpr float LABEL_HEIGHT = 30.0f;		///< 「Lv」「OVERDRIVE」の文字の高さ（ピクセル）
	static constexpr float LABEL_OFFSET_Y = 84.0f;		///< 「Lv」の文字の、数字の真ん中からの高さ（ピクセル）
	static constexpr int LEVEL_LENGTH = 1;				///< Lv の数字の文字数
	static constexpr int LABEL_LENGTH = 9;				///< 「OVERDRIVE」の文字数

	const BattleCharacter* m_player = nullptr;			///< 見るプレイヤー（借りているだけ）
	HudShape m_meter;									///< 弧・目盛り・針（Assets/shader/hudSpeedMeter.fx）
	HudText m_levelText;								///< Lv の数字
	HudText m_labelText;								///< 「Lv」の文字（OVERDRIVE 中は「OVERDRIVE」）
};
