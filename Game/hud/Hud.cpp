#include "stdafx.h"
#include "hud/Hud.h"

#include "actor/BattleCharacter.h"
#include "camera/ScreenPosition.h"
#include "gimmick/Anchor.h"
#include "gimmick/GiantFirewall.h"
#include "param/GameParam.h"
#include "stage/Checkpoint.h"

namespace
{
	// Vector4 は constexpr にできないので、クラスの定数ではなくここに置く
	const Vector4 LABEL_COLOR(0.0f, 0.90f, 1.0f, 1.0f);		///< 見出しのシアン（#00e5ff）
	const Vector4 VALUE_COLOR(1.0f, 1.0f, 1.0f, 1.0f);		///< 数字の白
	const Vector4 CROSSHAIR_COLOR(1.0f, 1.0f, 1.0f, 0.9f);	///< 照準の色
	const Vector4 GRAPPLE_COLOR(0.0f, 0.90f, 1.0f, 1.0f);	///< 角カッコと「[RB] GRAPPLE」のシアン
	const Vector4 SIGNAL_LOST_COLOR(1.0f, 0.2f, 0.25f, 1.0f);	///< 「SIGNAL LOST」の赤
	const Vector4 SIGNAL_LOST_GHOST_COLOR(0.5f, 0.0f, 0.1f, 0.8f);	///< 後ろに重ねる暗い赤
}

void Hud::Init(const BattleCharacter* player, const Checkpoint* checkpoint, const GiantFirewall* giantFirewall)
{
	m_player = player;
	m_checkpoint = checkpoint;
	m_giantFirewall = giantFirewall;
	m_time = 0.0f;

	m_speedMeter.Init(player);
	m_tutorial.Init(player);
	m_hitMarker.Init(player);
	m_crosshair.Init("Assets/shader/hudCrosshair.fx", CROSSHAIR_SIZE, CROSSHAIR_SIZE);
	m_bracket.Init("Assets/shader/hudBracket.fx", BRACKET_SIZE, BRACKET_SIZE);

	const char* timeLabel = "TIME";
	const char* checkpointLabel = "CHECKPOINT";
	const char* grappleText = "[RB] GRAPPLE";

	m_timeLabel.Init(static_cast<int>(strlen(timeLabel)), LABEL_HEIGHT, HudText::Align::Left);
	m_timeLabel.SetText(timeLabel);
	m_timeLabel.SetColor(LABEL_COLOR);
	m_timeValue.Init(TIME_LENGTH, VALUE_HEIGHT, HudText::Align::Left);
	m_timeValue.SetColor(VALUE_COLOR);

	m_checkpointLabel.Init(static_cast<int>(strlen(checkpointLabel)), LABEL_HEIGHT, HudText::Align::Right);
	m_checkpointLabel.SetText(checkpointLabel);
	m_checkpointLabel.SetColor(LABEL_COLOR);
	m_checkpointValue.Init(CHECKPOINT_LENGTH, VALUE_HEIGHT, HudText::Align::Right);
	m_checkpointValue.SetColor(VALUE_COLOR);

	m_grappleText.Init(static_cast<int>(strlen(grappleText)), GRAPPLE_TEXT_HEIGHT, HudText::Align::Left);
	m_grappleText.SetText(grappleText);
	m_grappleText.SetColor(GRAPPLE_COLOR);

	const char* signalLost = "SIGNAL LOST";
	m_signalLostScreen.Init("Assets/shader/hudSignalLost.fx", UI_SPACE_WIDTH, UI_SPACE_HEIGHT);
	m_signalLostText.Init(static_cast<int>(strlen(signalLost)), SIGNAL_LOST_HEIGHT, HudText::Align::Center);
	m_signalLostText.SetText(signalLost);
	m_signalLostText.SetColor(SIGNAL_LOST_COLOR);
	m_signalLostGhost.Init(static_cast<int>(strlen(signalLost)), SIGNAL_LOST_HEIGHT, HudText::Align::Center);
	m_signalLostGhost.SetText(signalLost);
	m_signalLostGhost.SetColor(SIGNAL_LOST_GHOST_COLOR);
}

void Hud::Update()
{
	// 案内はタイムが止まった後も進める（突破の後の案内を出し、出したものを消すため）
	m_tutorial.Update();
	// ヒットマークもタイムが止まった後も動かす（突破の後に倒しても出るように）
	m_hitMarker.Update();

	if (m_giantFirewall != nullptr && m_giantFirewall->IsBroken())
	{
		return;
	}
	const float dt = g_gameTime->GetFrameDeltaTime();
	m_time += dt;
	m_levelTimeSum += static_cast<float>(m_player->GetSpeedSystem().GetLevel()) * dt;
}

void Hud::Draw(RenderContext& rc)
{
	if (m_player == nullptr)
	{
		return;
	}

	m_crosshair.Update(Vector2::Zero, CROSSHAIR_COLOR, 0.0f);
	m_crosshair.Draw();
	m_hitMarker.Draw();

	DrawGrappleMarker();
	DrawInfo();
	m_speedMeter.Draw();
	m_tutorial.Draw(rc);
	// いちばん上に重ねる
	DrawSignalLost();
}

void Hud::DrawSignalLost()
{
	if (m_checkpoint == nullptr || !m_checkpoint->IsSignalLost())
	{
		return;
	}

	const float rate = m_checkpoint->GetSignalLostRate();
	m_signalLostScreen.Update(Vector2::Zero, Vector4::White, rate);
	m_signalLostScreen.Draw();

	// 出た瞬間ほど大きく左右に震わせる（1 秒に SIGNAL_LOST_SHAKE_STEPS 回、向きを変える）
	const float side = (static_cast<int>(rate * SIGNAL_LOST_SHAKE_STEPS) % 2 == 0) ? 1.0f : -1.0f;
	const float jitter = side * SIGNAL_LOST_JITTER * max(0.0f, 1.0f - rate * 3.0f);
	m_signalLostGhost.SetPosition(Vector2(jitter + SIGNAL_LOST_GHOST, -SIGNAL_LOST_GHOST * 0.5f));
	m_signalLostGhost.Draw();
	m_signalLostText.SetPosition(Vector2(jitter, 0.0f));
	m_signalLostText.Draw();
}


void Hud::DrawInfo()
{
	const float left = -(UI_SPACE_WIDTH * 0.5f) + SCREEN_MARGIN;
	const float right = UI_SPACE_WIDTH * 0.5f - SCREEN_MARGIN;
	const float top = UI_SPACE_HEIGHT * 0.5f - SCREEN_MARGIN;

	// タイム（分:秒.百分の一秒）
	const int minutes = static_cast<int>(m_time / SECONDS_PER_MINUTE);
	const float seconds = m_time - minutes * SECONDS_PER_MINUTE;
	char timeText[32];
	sprintf_s(timeText, "%02d:%05.2f", minutes, seconds);
	m_timeValue.SetText(timeText);

	m_timeLabel.SetPosition(Vector2(left, top));
	m_timeValue.SetPosition(Vector2(left, top - VALUE_OFFSET_Y));
	m_timeLabel.Draw();
	m_timeValue.Draw();

	// チェックポイント（今の番号 / 全部の数）
	if (m_checkpoint != nullptr)
	{
		const int total = static_cast<int>(nsGameParam::Get().stage.sections.size());
		char checkpointText[16];
		sprintf_s(checkpointText, "%d/%d", m_checkpoint->GetIndex() + 1, total);
		m_checkpointValue.SetText(checkpointText);

		m_checkpointLabel.SetPosition(Vector2(right, top));
		m_checkpointValue.SetPosition(Vector2(right, top - VALUE_OFFSET_Y));
		m_checkpointLabel.Draw();
		m_checkpointValue.Draw();
	}
}

void Hud::DrawGrappleMarker()
{
	const Anchor* anchor = m_player->GetLockedAnchor();
	if (anchor == nullptr)
	{
		return;
	}

	// カメラの後ろにあるときは出さない
	Vector2 screenPos;
	if (!nsScreen::CalcScreenPosition(anchor->GetPosition(), screenPos))
	{
		return;
	}

	m_bracket.Update(screenPos, GRAPPLE_COLOR, 0.0f);
	m_bracket.Draw();

	m_grappleText.SetPosition(Vector2(screenPos.x + GRAPPLE_TEXT_OFFSET_X, screenPos.y));
	m_grappleText.Draw();
}
