#pragma once
#include "hud/HudShape.h"
#include "hud/HudText.h"

class BattleCharacter;

/**
 * 画面右下の 4 分円のスピードメーター。針は今の Lv の中の進み具合で、振り切ると Lv が上がって根元に戻る
 * Lv5 で振り切ると OVERDRIVE になり、針が張り付いて縁と目盛りがシアンで光る。Hud が持つ
 * 内側の丸い計器の中に、プレイヤーの実際の速さ（移動速度の横の長さ）を数字で出す
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
	static constexpr float LEVEL_X_RATE = 0.15f;		///< Lv の数字の位置（右下の角から左へ、メーターの大きさに対する割合）
	static constexpr float LEVEL_Y_RATE = 0.60f;		///< Lv の数字の位置（右下の角から上へ）
	static constexpr float LEVEL_HEIGHT = 110.0f;		///< Lv の数字の高さ（ピクセル）
	static constexpr float LABEL_HEIGHT = 24.0f;		///< 「Lv」「OVERDRIVE」の文字の高さ（ピクセル）
	static constexpr float LABEL_GAP = 8.0f;			///< 「Lv」の文字の右端と、Lv の数字の左端の空き（ピクセル）
	static constexpr float LABEL_Y_RATE = 0.50f;		///< 「Lv」の文字の高さの位置（右下の角から上へ。内側の丸い計器の上の端より上にする）
	static constexpr float SPEED_X_RATE = 0.30f;		///< 速度の数字の位置（hudSpeedMeter.fx の DIAL_CENTER と同じにする）
	static constexpr float SPEED_Y_RATE = 0.26f;		///< 速度の数字の位置（同上）
	static constexpr float SPEED_HEIGHT = 38.0f;		///< 速度の数字の高さ（ピクセル。4 桁が丸の帯の内側に収まる大きさ）
	static constexpr float SPEED_MAX_SHOW = 9999.0f;	///< 出せる速度の上限（SPEED_LENGTH 桁に収める）
	static constexpr int LEVEL_LENGTH = 1;				///< Lv の数字の文字数
	static constexpr int LABEL_LENGTH = 9;				///< 「OVERDRIVE」の文字数
	static constexpr int SPEED_LENGTH = 4;				///< 速度の数字の桁数

	const BattleCharacter* m_player = nullptr;			///< 見るプレイヤー（借りているだけ）
	HudShape m_meter;									///< 縁・目盛り・針・内側の丸い計器（Assets/shader/hudSpeedMeter.fx）
	HudText m_levelText;								///< Lv の数字
	HudText m_labelText;								///< 「Lv」の文字（OVERDRIVE 中は「OVERDRIVE」）。Lv の数字の左に右端をそろえて出す
	HudText m_speedText;								///< 内側の丸い計器の中の、実際の速さ（横の速さ。デバッグ表示の「横の速さ」と同じ）
};
