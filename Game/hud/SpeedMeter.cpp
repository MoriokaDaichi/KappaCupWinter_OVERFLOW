#include "stdafx.h"
#include "hud/SpeedMeter.h"

#include "actor/BattleCharacter.h"

namespace
{
	// Vector4 は constexpr にできないので、クラスの定数ではなくここに置く
	const Vector4 LEVEL_COLOR(1.0f, 1.0f, 1.0f, 1.0f);			///< Lv の数字の色
	const Vector4 LABEL_COLOR(0.71f, 0.30f, 1.0f, 1.0f);		///< 「Lv」の文字の紫（#b44dff）
	const Vector4 OVERDRIVE_COLOR(0.0f, 0.90f, 1.0f, 1.0f);		///< OVERDRIVE 中の文字のシアン（#00e5ff）
}

void SpeedMeter::Init(const BattleCharacter* player)
{
	m_player = player;
	m_meter.Init("Assets/shader/hudSpeedMeter.fx", METER_SIZE, METER_SIZE);
	m_levelText.Init(LEVEL_LENGTH, LEVEL_HEIGHT, HudText::Align::Center);
	m_levelText.SetColor(LEVEL_COLOR);
	m_labelText.Init(LABEL_LENGTH, LABEL_HEIGHT, HudText::Align::Center);
}

void SpeedMeter::Draw()
{
	if (m_player == nullptr)
	{
		return;
	}

	const SpeedSystem& speed = m_player->GetSpeedSystem();
	const bool isOverdrive = speed.IsOverdrive();

	// 右下の角
	const float cornerX = UI_SPACE_WIDTH * 0.5f - SCREEN_MARGIN;
	const float cornerY = -(UI_SPACE_HEIGHT * 0.5f - SCREEN_MARGIN);

	const Vector2 meterCenter(cornerX - METER_SIZE * 0.5f, cornerY + METER_SIZE * 0.5f);
	m_meter.Update(meterCenter, LEVEL_COLOR, isOverdrive ? 1.0f : speed.GetLevelRate(), isOverdrive ? 1.0f : 0.0f);
	m_meter.Draw();

	const Vector2 levelPos(cornerX - METER_SIZE * LEVEL_X_RATE, cornerY + METER_SIZE * LEVEL_Y_RATE);
	char levelText[8];
	sprintf_s(levelText, "%d", speed.GetLevel());
	m_levelText.SetText(levelText);
	m_levelText.SetPosition(levelPos);
	m_levelText.Draw();

	m_labelText.SetText(isOverdrive ? "OVERDRIVE" : "Lv");
	m_labelText.SetColor(isOverdrive ? OVERDRIVE_COLOR : LABEL_COLOR);
	m_labelText.SetPosition(Vector2(levelPos.x, levelPos.y + LABEL_OFFSET_Y));
	m_labelText.Draw();
}
