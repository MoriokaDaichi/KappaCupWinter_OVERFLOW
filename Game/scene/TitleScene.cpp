#include "stdafx.h"
#include "scene/TitleScene.h"

#include "Game.h"
#include "input/InputManager.h"
#include "scene/Fade.h"

namespace
{
	// Vector4 は constexpr にできないので、クラスの定数ではなくここに置く
	const Vector4 GRID_COLOR(0.0f, 0.9f, 1.0f, 1.0f);		///< 背景の格子のシアン
	const Vector4 LOGO_COLOR(0.0f, 0.9f, 1.0f, 1.0f);		///< タイトルの文字のシアン
	const Vector4 SUB_COLOR(1.0f, 0.17f, 0.84f, 1.0f);		///< 小さい文字のマゼンタ
	const Vector4 PRESS_COLOR(1.0f, 1.0f, 1.0f, 1.0f);		///< 「PRESS [A] START」の白
	const Vector4 KEY_COLOR(0.6f, 0.7f, 0.8f, 1.0f);		///< キーボードの案内の灰色
}

bool TitleScene::Start()
{
	m_fade = FindGO<Fade>(Fade::NAME);
	if (m_fade != nullptr)
	{
		m_fade->StartFadeIn(FADE_TIME);
	}

	m_back.Init("Assets/shader/sceneBack.fx", UI_SPACE_WIDTH, UI_SPACE_HEIGHT);

	// HudText は 1 つに 16 文字まで
	const char* logo = "OVERFLOW";
	const char* sub = "FULL SPEED HACK";
	const char* press = "PRESS [A] START";
	const char* key = "KEY: ENTER / J";

	m_logo.Init(static_cast<int>(strlen(logo)), LOGO_HEIGHT, HudText::Align::Center);
	m_logo.SetText(logo);
	m_logo.SetColor(LOGO_COLOR);
	m_logo.SetPosition(Vector2(0.0f, LOGO_Y));

	m_sub.Init(static_cast<int>(strlen(sub)), SUB_HEIGHT, HudText::Align::Center);
	m_sub.SetText(sub);
	m_sub.SetColor(SUB_COLOR);
	m_sub.SetPosition(Vector2(0.0f, SUB_Y));

	m_press.Init(static_cast<int>(strlen(press)), PRESS_HEIGHT, HudText::Align::Center);
	m_press.SetText(press);
	m_press.SetPosition(Vector2(0.0f, PRESS_Y));

	m_key.Init(static_cast<int>(strlen(key)), KEY_HEIGHT, HudText::Align::Center);
	m_key.SetText(key);
	m_key.SetColor(KEY_COLOR);
	m_key.SetPosition(Vector2(0.0f, KEY_Y));

	return true;
}

void TitleScene::Update()
{
	InputManager& input = InputManager::GetInstance();
	input.Update();
	m_time += g_gameTime->GetFrameDeltaTime();

	if (!m_isLeaving && (input.IsTrigger(EnInputAction::Jump) || input.IsTrigger(EnInputAction::Pause)))
	{
		m_isLeaving = true;
		if (m_fade != nullptr)
		{
			m_fade->StartFadeOut(FADE_TIME);
		}
	}

	// 真っ黒になってからゲームを作る（ゲームを作るときは読み込みで少し止まるので、その間を黒で隠す）
	if (m_isLeaving && (m_fade == nullptr || m_fade->IsBlack()))
	{
		NewGO<Game>(0, "game");
		DeleteGO(this);
	}
}

void TitleScene::Render(RenderContext& rc)
{
	m_back.Update(Vector2::Zero, GRID_COLOR, GRID_SPEED);
	m_back.Draw();

	m_logo.Draw();
	m_sub.Draw();

	// 押されたら点滅をやめて、つけたままにする（押したことが伝わるように）
	const float blink = m_isLeaving ? 1.0f : 0.55f + 0.45f * sinf(m_time * BLINK_SPEED);
	m_press.SetColor(Vector4(PRESS_COLOR.x, PRESS_COLOR.y, PRESS_COLOR.z, blink));
	m_press.Draw();
	m_key.Draw();
}
