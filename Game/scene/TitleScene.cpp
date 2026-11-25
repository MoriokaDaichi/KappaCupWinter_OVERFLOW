#include "stdafx.h"
#include "scene/TitleScene.h"

#include "Game.h"
#include "input/InputManager.h"
#include "scene/Fade.h"
#include <cstdlib>

namespace
{
	// Vector4 は constexpr にできないので、クラスの定数ではなくここに置く
	const Vector4 CODE_COLOR(0.25f, 1.0f, 0.6f, 0.75f);		///< コードの緑
	const Vector4 ALERT_COLOR(1.0f, 0.17f, 0.84f, 1.0f);	///< 「BUFFER OVERFLOW DETECTED」のマゼンタ
	const Vector4 LOGO_COLOR(1.0f, 1.0f, 1.0f, 1.0f);		///< ロゴの白
	const Vector4 MAGENTA(1.0f, 0.17f, 0.84f, 0.8f);		///< ロゴの色ずれのマゼンタ
	const Vector4 CYAN(0.0f, 0.9f, 1.0f, 0.8f);				///< ロゴの色ずれのシアン
	const Vector4 PRESS_COLOR(0.0f, 0.9f, 1.0f, 1.0f);		///< 「> PRESS START_」のシアン
	const Vector4 KEY_COLOR(0.5f, 0.55f, 0.7f, 1.0f);		///< キーボードの案内の灰色

	/** 画面の中のコード（企画書の絵と同じ並び。HudText は 1 行 32 文字まで）。0〜3 が上、4〜7 が下 */
	const char* CODE_LINES[] = {
		"0X7FFE 4A3B C91D 00FF E2A1 7C0D",
		"> CONNECT --NODE 192.168.0.13 OK",
		"> SCAN FIREWALL.LAYER[0..5]",
		"[####------] 41% BUF=0X00A4",
		"> INJECT KINETIC_SUIT.DLL",
		"WRITE 0XFFFF BYTES -> BUF[256]",
		"!! BUFFER OVERFLOW DETECTED !!",
		"4F 9C 11 FE 00 7B 2D 88 C4 1E",
	};
	constexpr int HEX_TOP_LINE = 0;			///< 数字が変わり続ける行（上）
	constexpr int PROGRESS_LINE = 3;		///< 進み具合の棒の行
	constexpr int ALERT_LINE = 6;			///< マゼンタにする行
	constexpr int HEX_BOTTOM_LINE = 7;		///< 数字が変わり続ける行（下）

	/** 端末のまわりに浮かぶ数字（企画書の絵の 0・1・F・7）と、その位置 */
	const char* FLOAT_DIGITS[] = { "0", "F", "7", "0", "1", "1" };
	const Vector2 FLOAT_DIGIT_POSITIONS[] = {
		Vector2(-560.0f, 420.0f), Vector2(-780.0f, 330.0f), Vector2(420.0f, 470.0f),
		Vector2(640.0f, 440.0f), Vector2(560.0f, 390.0f), Vector2(820.0f, -120.0f),
	};

	/** 0〜9 と A〜F から 1 文字 */
	char RandomHex()
	{
		const char* HEX = "0123456789ABCDEF";
		return HEX[std::rand() % 16];
	}
}

bool TitleScene::Start()
{
	static_assert(ARRAYSIZE(CODE_LINES) == CODE_LINE_NUM, "CODE_LINES の数を CODE_LINE_NUM と合わせてください");
	static_assert(ARRAYSIZE(FLOAT_DIGITS) == FLOAT_DIGIT_NUM, "FLOAT_DIGITS の数を FLOAT_DIGIT_NUM と合わせてください");

	m_fade = FindGO<Fade>(Fade::NAME);
	if (m_fade != nullptr)
	{
		m_fade->StartFadeIn(FADE_IN_TIME);
	}

	m_device.Init("Assets/shader/titleDevice.fx", UI_SPACE_WIDTH, UI_SPACE_HEIGHT);
	m_glitch.Init("Assets/shader/titleGlitch.fx", GLITCH_WIDTH, GLITCH_HEIGHT);
	m_dive.Init("Assets/shader/titleDive.fx", UI_SPACE_WIDTH, UI_SPACE_HEIGHT);

	for (int i = 0; i < CODE_LINE_NUM; i++)
	{
		m_codeText[i] = CODE_LINES[i];
		m_code[i].Init(HudText::MAX_LENGTH, CODE_HEIGHT, HudText::Align::Left);
		m_code[i].SetText("");
	}

	const char* logo = "OVERFLOW";
	HudText* logos[] = { &m_logoMagenta, &m_logoCyan, &m_logo };
	const Vector4 logoColors[] = { MAGENTA, CYAN, LOGO_COLOR };
	for (int i = 0; i < 3; i++)
	{
		logos[i]->Init(static_cast<int>(strlen(logo)), LOGO_HEIGHT, HudText::Align::Center);
		logos[i]->SetText(logo);
		logos[i]->SetColor(logoColors[i]);
	}

	const char* press = "> PRESS START_";
	m_press.Init(static_cast<int>(strlen(press)), PRESS_HEIGHT, HudText::Align::Center);
	m_press.SetColor(PRESS_COLOR);

	const char* key = "KEY: ENTER / J";
	m_key.Init(static_cast<int>(strlen(key)), KEY_HEIGHT, HudText::Align::Center);
	m_key.SetText(key);
	m_key.SetColor(KEY_COLOR);

	for (int i = 0; i < FLOAT_DIGIT_NUM; i++)
	{
		m_floatDigits[i].Init(1, FLOAT_DIGIT_HEIGHT, HudText::Align::Center);
		m_floatDigits[i].SetText(FLOAT_DIGITS[i]);
	}
	for (int i = 0; i < DIVE_DIGIT_NUM; i++)
	{
		m_diveDigits[i].Init(1, DIVE_DIGIT_HEIGHT, HudText::Align::Center);
		const char text[2] = { (i % 3 == 0) ? 'F' : static_cast<char>('0' + i % 2), '\0' };
		m_diveDigits[i].SetText(text);
	}

	return true;
}

void TitleScene::ChangePhase(const Phase phase)
{
	m_phase = phase;
	m_phaseTime = 0.0f;
}

void TitleScene::Update()
{
	InputManager& input = InputManager::GetInstance();
	input.Update();
	const float dt = g_gameTime->GetFrameDeltaTime();
	m_time += dt;
	m_phaseTime += dt;

	UpdateCode();

	switch (m_phase)
	{
	case Phase::Idle:
		if (input.IsTrigger(EnInputAction::Jump) || input.IsTrigger(EnInputAction::Pause))
		{
			ChangePhase(Phase::Tap);
		}
		break;
	case Phase::Tap:
		if (m_phaseTime >= TAP_TIME)
		{
			ChangePhase(Phase::Flash);
		}
		break;
	case Phase::Flash:
		if (m_phaseTime >= FLASH_TIME)
		{
			ChangePhase(Phase::Dive);
		}
		break;
	case Phase::Dive:
		if (m_phaseTime >= DIVE_TIME)
		{
			ChangePhase(Phase::Leave);
			if (m_fade != nullptr)
			{
				m_fade->StartFadeOut(LEAVE_TIME);
			}
		}
		break;
	case Phase::Leave:
		// 真っ黒になったら、まず「> DIVING...」を 1 フレーム出してから、次のフレームでゲームを作る
		// （ゲームを作ると読み込みで数秒止まるので、その間に出ている絵を、真っ黒ではなく文字にする）
		if (m_fade != nullptr && !m_fade->IsBlack())
		{
			break;
		}
		if (!m_isLoadingShown)
		{
			m_isLoadingShown = true;
			if (m_fade != nullptr)
			{
				m_fade->SetMessage("> DIVING...");
			}
			break;
		}
		if (!m_isGame)
		{
			m_isGame = true;
			NewGO<Game>(0, "game");
			DeleteGO(this);
		}
		break;
	}
}

void TitleScene::UpdateCode()
{
	// 変わり続ける行（16 進の数字と、進み具合の棒）を作り直す
	m_codeRefreshTimer -= g_gameTime->GetFrameDeltaTime();
	if (m_codeRefreshTimer <= 0.0f)
	{
		m_codeRefreshTimer = CODE_REFRESH_TIME;
		for (const int line : { HEX_TOP_LINE, HEX_BOTTOM_LINE })
		{
			std::string& text = m_codeText[line];
			for (size_t i = 0; i < text.size(); i++)
			{
				// 空白と「0X」（16 進の印）はそのままにする
				const bool isPrefix = text[i] == 'X' || (i + 1 < text.size() && text[i + 1] == 'X');
				if (text[i] != ' ' && !isPrefix)
				{
					text[i] = RandomHex();
				}
			}
		}

		const int percent = 41 + static_cast<int>(m_time * 23.0f) % 59;
		const int filled = percent / 10;
		char progress[64];
		sprintf_s(progress, "[%.*s%.*s] %d%% BUF=0X%c%c%c%c", filled, "##########", 10 - filled, "----------",
			percent, RandomHex(), RandomHex(), RandomHex(), RandomHex());
		m_codeText[PROGRESS_LINE] = progress;
	}

	// 行ごとに少し遅れて、1 文字ずつ打ち込むように出す
	for (int i = 0; i < CODE_LINE_NUM; i++)
	{
		const float typed = (m_time - CODE_LINE_DELAY * i) * CODE_TYPE_SPEED;
		const int length = max(0, min(static_cast<int>(typed), static_cast<int>(m_codeText[i].size())));
		m_code[i].SetText(m_codeText[i].substr(0, length).c_str());
	}
}

void TitleScene::DrawPanel(const float alpha)
{
	if (alpha <= 0.0f)
	{
		return;
	}

	for (int i = 0; i < CODE_LINE_NUM; i++)
	{
		const bool isTop = i < CODE_LINE_NUM / 2;
		const float y = (isTop ? CODE_TOP_Y : CODE_BOTTOM_Y) - CODE_SPACING * (i % (CODE_LINE_NUM / 2));
		const Vector4& color = (i == ALERT_LINE) ? ALERT_COLOR : CODE_COLOR;
		m_code[i].SetColor(Vector4(color.x, color.y, color.z, color.w * alpha));
		m_code[i].SetPosition(Vector2(CODE_LEFT, y));
		m_code[i].Draw();
	}

	// ロゴ: ふだんは色が少しずれ、ときどき大きく乱れる
	const bool isGlitch = fmodf(m_time, GLITCH_INTERVAL) < GLITCH_TIME || m_phase == Phase::Tap;
	const float ghost = isGlitch ? LOGO_GLITCH_GHOST * (static_cast<float>(std::rand() % 100) / 100.0f) : LOGO_GHOST;
	const float shake = isGlitch ? static_cast<float>(std::rand() % 9 - 4) : 0.0f;
	m_logoMagenta.SetColor(Vector4(MAGENTA.x, MAGENTA.y, MAGENTA.z, MAGENTA.w * alpha));
	m_logoCyan.SetColor(Vector4(CYAN.x, CYAN.y, CYAN.z, CYAN.w * alpha));
	m_logo.SetColor(Vector4(LOGO_COLOR.x, LOGO_COLOR.y, LOGO_COLOR.z, alpha));
	m_logoMagenta.SetPosition(Vector2(-ghost + shake, LOGO_Y));
	m_logoCyan.SetPosition(Vector2(ghost + shake, LOGO_Y));
	m_logo.SetPosition(Vector2(shake, LOGO_Y));
	m_logoMagenta.Draw();
	m_logoCyan.Draw();
	m_logo.Draw();

	m_glitch.Update(Vector2(0.0f, LOGO_Y), Vector4(1.0f, 1.0f, 1.0f, alpha), isGlitch ? 1.0f : 0.0f);
	m_glitch.Draw();

	// 「> PRESS START_」の「_」はカーソルのように点滅する。押したら付けたままにする
	const bool isCursorOn = m_phase != Phase::Idle || fmodf(m_time, CURSOR_BLINK_TIME * 2.0f) < CURSOR_BLINK_TIME;
	m_press.SetText(isCursorOn ? "> PRESS START_" : "> PRESS START");
	m_press.SetColor(Vector4(PRESS_COLOR.x, PRESS_COLOR.y, PRESS_COLOR.z, alpha));
	m_press.SetPosition(Vector2(0.0f, PRESS_Y));
	m_press.Draw();
}

void TitleScene::DrawFloatingDigits()
{
	for (int i = 0; i < FLOAT_DIGIT_NUM; i++)
	{
		// ゆっくり上下にただよい、ちらつく。色は緑とマゼンタを混ぜる（企画書の絵）
		const float phase = m_time * 1.3f + i * 1.7f;
		const Vector2 position(FLOAT_DIGIT_POSITIONS[i].x, FLOAT_DIGIT_POSITIONS[i].y + sinf(phase) * 10.0f);
		const float flicker = 0.5f + 0.5f * sinf(phase * 2.3f);
		const Vector4 color = (i == 2) ? ALERT_COLOR : CODE_COLOR;
		m_floatDigits[i].SetColor(Vector4(color.x, color.y, color.z, 0.4f + 0.5f * flicker));
		m_floatDigits[i].SetPosition(position);
		m_floatDigits[i].Draw();
	}
}

void TitleScene::DrawDiveDigits()
{
	// 真ん中の近くから外へ、だんだん速く飛んでいく（自分が落ちていく速さに見せる）
	for (int i = 0; i < DIVE_DIGIT_NUM; i++)
	{
		const float angle = i * 2.4f;
		const float cycle = fmodf(m_phaseTime * 0.8f + i * 0.37f, 1.0f);
		const float distance = 80.0f + cycle * cycle * 900.0f;
		const Vector2 position(cosf(angle) * distance * 1.6f, sinf(angle) * distance);
		const float scale = 0.6f + cycle * 1.4f;
		const Vector4& color = (i % 4 == 0) ? ALERT_COLOR : CODE_COLOR;
		m_diveDigits[i].SetColor(Vector4(color.x, color.y, color.z, min(cycle * 3.0f, 1.0f)));
		m_diveDigits[i].SetPosition(position, scale);
		m_diveDigits[i].Draw();
	}
}

void TitleScene::Render(RenderContext& rc)
{
	if (m_phase == Phase::Dive || m_phase == Phase::Leave)
	{
		const float progress = min(m_phaseTime / DIVE_TIME, 1.0f);
		m_dive.Update(Vector2::Zero, Vector4::White, m_phase == Phase::Leave ? 1.0f : progress);
		m_dive.Draw();
		DrawDiveDigits();
		return;
	}

	// 親指で押し込む量と、画面が白く光る量
	const float press = (m_phase == Phase::Tap) ? min(m_phaseTime / TAP_TIME, 1.0f) : (m_phase == Phase::Flash ? 1.0f : 0.0f);
	const float flash = (m_phase == Phase::Flash) ? min(m_phaseTime / FLASH_TIME, 1.0f) : 0.0f;
	m_device.Update(Vector2::Zero, Vector4::White, press, flash);
	m_device.Draw();

	DrawPanel(1.0f - flash);
	DrawFloatingDigits();

	if (m_phase == Phase::Idle)
	{
		m_key.SetPosition(Vector2(0.0f, KEY_Y));
		m_key.Draw();
	}
}
