#include "stdafx.h"
#include "scene/TitleScene.h"

#include "Game.h"
#include "input/InputManager.h"
#include "camera/ScreenPosition.h"
#include "scene/Fade.h"
#include "scene/SceneExposure.h"
#include <cfloat>
#include <cstdlib>

namespace
{
	// Vector4 は constexpr にできないので、クラスの定数ではなくここに置く
	const Vector4 CODE_COLOR(0.25f, 1.0f, 0.6f, 0.75f);		///< コードの緑
	const Vector4 ALERT_COLOR(1.0f, 0.17f, 0.84f, 1.0f);	///< 「BUFFER OVERFLOW DETECTED」のマゼンタ
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

namespace
{
	/**
	 * Blender（D:\my\05_Assets\02_OVERFLOW\title の pose_title.py）で決めた、端末の位置と向き（目を原点にした 3ds Max の座標。cm）
	 * ゲームの座標へは ToEngine で直す（モデルの上は Z。エンジンが読むと Y が上、-Y が前の +Z になる）
	 */
	const Vector3 TABLET_CENTER(0.0f, -34.0f, -20.0f);		///< 端末の中心
	const Vector3 TABLET_RIGHT(-1.0f, 0.0f, 0.0f);			///< 見ている人の右
	const Vector3 TABLET_UP(0.0f, -0.507f, 0.862f);			///< 画面の上
	const Vector3 TABLET_NORMAL(0.0f, 0.862f, 0.507f);		///< 画面の向き（目の方）
	constexpr float SCREEN_HALF_WIDTH = 11.5f;				///< 画面の大きさの半分
	constexpr float SCREEN_HALF_HEIGHT = 6.8f;
	constexpr float SCREEN_FRONT = 0.7f;					///< 画面の、端末の中心からの前への出っぱり（厚さの半分）
	constexpr float THUMB_FROM_EDGE = 12.7f;				///< 親指の先の、端末の中心からの横のずれ
	constexpr float THUMB_DOWN = 2.0f;						///< 親指の先の、中心からの下へのずれ
	constexpr float THUMB_FRONT = 1.8f;						///< 親指の先の、前への出っぱり
	constexpr float Z_UP_ROTATION = -90.0f;					///< Z が上のモデルを Y が上に起こす回転（X 軸のまわり、度）。エンジンの enModelUpAxisZ と同じ

	/** 3ds Max の座標（Z が上、-Y が前）を、ゲームの座標（Y が上、+Z が前）に直す */
	Vector3 ToEngine(const Vector3& v)
	{
		return Vector3(v.x, v.z, -v.y);
	}
}

void TitleScene::InitModels()
{
	// カメラは主人公の目（原点）。端末の真ん中を見る
	g_camera3D->SetPosition(Vector3::Zero);
	g_camera3D->SetTarget(ToEngine(TABLET_CENTER));
	g_camera3D->SetUp(Vector3::Up);
	g_camera3D->SetViewAngle(Math::DegToRad(CAMERA_VIEW_ANGLE));
	g_camera3D->SetNear(CAMERA_NEAR);
	g_camera3D->SetFar(CAMERA_FAR);
	g_camera3D->Update();

	nsSceneExposure::SetScale(EXPOSURE_SCALE);
	Vector3 lightDirection(0.3f, -1.0f, 0.6f);
	lightDirection.Normalize();
	g_renderingEngine->SetDirectionLight(0, lightDirection, Vector3(LIGHT_BRIGHTNESS, LIGHT_BRIGHTNESS, LIGHT_BRIGHTNESS));

	// 3 つとも、モデルは原点（目）から離れた所にある。エンジンに「Z が上」と渡すと、forward 描画のモデルは
	// 画面に入るかの判定（ビューカリング）にだけ Z を起こす回転が入らず、カメラの後ろにあるとみなされて描かれない
	// （判定は読み込んでいない別の Model の行列を使い、その Model の上方向の初期値が Y のため）。
	// そこで「Y が上」として読み、Z を起こす回転（X 軸のまわりに -90 度）は自分で SetRotation で掛ける
	Quaternion zUpToYUp;
	zUpToYUp.SetRotationDegX(Z_UP_ROTATION);

	// 腕と手は、ゲームの主人公と同じテクスチャ（player フォルダ）に、上からの光とふちのシアンの光を足して描く（titleArms.fx）
	ModelInitData armsData;
	armsData.m_tkmFilePath = "Assets/modelData/player/title_arms.tkm";
	armsData.m_fxFilePath = "Assets/shader/titleArms.fx";
	armsData.m_colorBufferFormat[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
	armsData.m_modelUpAxis = enModelUpAxisY;
	m_arms.InitForwardRendering(armsData);
	m_arms.SetShadowCasterFlag(false);
	m_arms.SetRotation(zUpToYUp);
	m_arms.Update();

	ModelInitData tabletData;
	tabletData.m_tkmFilePath = "Assets/modelData/title/title_tablet.tkm";
	tabletData.m_fxFilePath = "Assets/shader/titleTablet.fx";
	tabletData.m_colorBufferFormat[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
	tabletData.m_modelUpAxis = enModelUpAxisY;
	m_tablet.InitForwardRendering(tabletData);
	m_tablet.SetShadowCasterFlag(false);
	m_tablet.SetRotation(zUpToYUp);
	m_tablet.Update();

	ModelInitData backData;
	backData.m_tkmFilePath = "Assets/modelData/title/title_back.tkm";
	backData.m_fxFilePath = "Assets/shader/titleBack.fx";
	backData.m_expandConstantBuffer = &m_backConstant;
	backData.m_expandConstantBufferSize = sizeof(m_backConstant);
	backData.m_colorBufferFormat[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
	// 板の表と裏のどちらがカメラを向いていても描く
	backData.m_cullMode = D3D12_CULL_MODE_NONE;
	backData.m_modelUpAxis = enModelUpAxisY;
	m_back.InitForwardRendering(backData);
	m_back.SetShadowCasterFlag(false);
	m_back.SetRotation(zUpToYUp);
	m_back.Update();
}

void TitleScene::CalcPanelRect()
{
	// 画面の四隅を、画面の上の位置に直して、囲む四角を出す
	const Vector3 front = TABLET_CENTER + TABLET_NORMAL * SCREEN_FRONT;
	Vector2 minPos(FLT_MAX, FLT_MAX);
	Vector2 maxPos(-FLT_MAX, -FLT_MAX);
	for (const float sx : { -1.0f, 1.0f })
	{
		for (const float sy : { -1.0f, 1.0f })
		{
			const Vector3 corner = front + TABLET_RIGHT * (sx * SCREEN_HALF_WIDTH) + TABLET_UP * (sy * SCREEN_HALF_HEIGHT);
			Vector2 screenPos;
			nsScreen::CalcScreenPosition(ToEngine(corner), screenPos);
			minPos.x = min(minPos.x, screenPos.x); minPos.y = min(minPos.y, screenPos.y);
			maxPos.x = max(maxPos.x, screenPos.x); maxPos.y = max(maxPos.y, screenPos.y);
		}
	}
	m_panelCenter = Vector2((minPos.x + maxPos.x) * 0.5f, (minPos.y + maxPos.y) * 0.5f);
	m_panelSize = Vector2(maxPos.x - minPos.x, maxPos.y - minPos.y);

	// 親指の先は左右に 1 つずつある。画面の右に映るほうに輪を出す
	m_touchPosition = Vector2(-FLT_MAX, 0.0f);
	for (const float side : { -1.0f, 1.0f })
	{
		const Vector3 thumb = TABLET_CENTER + TABLET_RIGHT * (side * THUMB_FROM_EDGE) - TABLET_UP * THUMB_DOWN + TABLET_NORMAL * THUMB_FRONT;
		Vector2 screenPos;
		nsScreen::CalcScreenPosition(ToEngine(thumb), screenPos);
		if (screenPos.x > m_touchPosition.x)
		{
			m_touchPosition = screenPos;
		}
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

	InitModels();
	CalcPanelRect();
	m_panel.Init("Assets/shader/titlePanel.fx", m_panelSize.x, m_panelSize.y);
	m_touch.Init("Assets/shader/titleTouch.fx", TOUCH_SIZE, TOUCH_SIZE);
	m_glitch.Init("Assets/shader/titleGlitch.fx", GLITCH_WIDTH, GLITCH_HEIGHT);
	m_dive.Init("Assets/shader/titleDive.fx", UI_SPACE_WIDTH, UI_SPACE_HEIGHT);
	m_dive.SetRealTime(true);

	for (int i = 0; i < CODE_LINE_NUM; i++)
	{
		m_codeText[i] = CODE_LINES[i];
		m_code[i].Init(HudText::MAX_LENGTH, CODE_HEIGHT, HudText::Align::Left);
		m_code[i].SetText("");
	}

	m_logo.Init("Assets/sprite/title/logo_final.DDS", LOGO_WIDTH, LOGO_HEIGHT);
	m_logoGlitch.Init("Assets/sprite/title/logo_glitch.DDS", LOGO_WIDTH, LOGO_HEIGHT);

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

	// ゲームで使う重い tkm を、裏で読み始める
	// タイトルのモデル（InitModels）を作り終えてから始める（tkm を 2 つのスレッドで同時に読まないため）
	m_prefetcher.Start();

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
	const float dt = g_gameTime->GetRealFrameDeltaTime();
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
		// 先読みが終わっていたら、ゲームを作り始める（ここから先は、トンネルの裏で 1 フレームに 1 段ずつ作られる）
		// 起動してすぐ押すと、まだ先読みが終わっていないので、毎フレーム見て、終わった次のフレームで作る
		if (m_game == nullptr && m_prefetcher.IsDone())
		{
			// 読んだ tkm をバンクに登録してから、ゲームを作る（ゲームはバンクから借りるだけになる）
			m_prefetcher.RegistToBank();
			m_game = NewGO<Game>(0, "game");
		}
		// 落ちる演出を最後まで見せて、かつ ゲームの準備ができたら、黒くし始める
		// 準備がまだなら Dive のまま（トンネルの輪は time で流れ続けるので、落ち続けて見える）
		if (m_phaseTime >= DIVE_TIME && m_game != nullptr && m_game->IsReady())
		{
			ChangePhase(Phase::Leave);
			if (m_fade != nullptr)
			{
				m_fade->StartFadeOut(LEAVE_TIME);
			}
		}
		break;
	case Phase::Leave:
		// 真っ黒になるまで待つ（ゲームに切り替わる瞬間を見せないため）
		if (m_fade != nullptr && !m_fade->IsBlack())
		{
			break;
		}
		// 遊び始める（読み込みの間止めていた時間を戻し、ゲームが黒から戻し始める）
		m_game->Begin();
		DeleteGO(this);
		break;
	}
}

void TitleScene::UpdateCode()
{
	// 変わり続ける行（16 進の数字と、進み具合の棒）を作り直す
	m_codeRefreshTimer -= g_gameTime->GetRealFrameDeltaTime();
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

void TitleScene::DrawPanel(RenderContext& rc, const float alpha)
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
		m_code[i].SetPosition(Vector2(m_panelCenter.x + CODE_LEFT, m_panelCenter.y + y));
		m_code[i].Draw();
	}

	// ロゴ: ふだんは光る本番の画像。ときどき（と、押した瞬間）色ずれのグリッチ版に切り替え、左右に震わせて帯を重ねる
	const bool isGlitch = fmodf(m_time, GLITCH_INTERVAL) < GLITCH_TIME || m_phase == Phase::Tap;
	SpriteRender& logo = isGlitch ? m_logoGlitch : m_logo;
	const float shake = isGlitch ? static_cast<float>(std::rand() % (LOGO_SHAKE * 2 + 1) - LOGO_SHAKE) : 0.0f;
	const float scale = isGlitch ? LOGO_GLITCH_SCALE : 1.0f;
	logo.SetMulColor(Vector4(1.0f, 1.0f, 1.0f, alpha));
	logo.SetPosition(Vector3(m_panelCenter.x + shake, m_panelCenter.y + LOGO_Y, 0.0f));
	logo.SetScale(Vector3(scale, scale, 1.0f));
	logo.Update();
	logo.Draw(rc);

	if (isGlitch)
	{
		m_glitch.Update(Vector2(m_panelCenter.x, m_panelCenter.y + LOGO_Y), Vector4(1.0f, 1.0f, 1.0f, alpha), 1.0f);
		m_glitch.Draw();
	}

	// 「> PRESS START_」の「_」はカーソルのように点滅する。押したら付けたままにする
	const bool isCursorOn = m_phase != Phase::Idle || fmodf(m_time, CURSOR_BLINK_TIME * 2.0f) < CURSOR_BLINK_TIME;
	m_press.SetText(isCursorOn ? "> PRESS START_" : "> PRESS START");
	m_press.SetColor(Vector4(PRESS_COLOR.x, PRESS_COLOR.y, PRESS_COLOR.z, alpha));
	m_press.SetPosition(Vector2(m_panelCenter.x, m_panelCenter.y + PRESS_Y));
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

	// 3D: 背景・端末・腕
	m_backConstant.time = m_time;
	m_back.Draw(rc);
	m_tablet.Draw(rc);
	m_arms.Draw(rc);

	// 2D: 端末の画面（押した後は白く光る）と、親指のまわりの輪
	const float press = (m_phase == Phase::Tap) ? min(m_phaseTime / TAP_TIME, 1.0f) : (m_phase == Phase::Flash ? 1.0f : 0.0f);
	const float flash = (m_phase == Phase::Flash) ? min(m_phaseTime / FLASH_TIME, 1.0f) : 0.0f;
	m_panel.Update(m_panelCenter, Vector4::White, 0.0f, flash, m_panelSize.y / max(m_panelSize.x, 1.0f));
	m_panel.Draw();

	DrawPanel(rc, 1.0f - flash);
	m_touch.Update(m_touchPosition, Vector4::White, press);
	m_touch.Draw();
	DrawFloatingDigits();

	if (m_phase == Phase::Idle)
	{
		m_key.SetPosition(Vector2(0.0f, KEY_Y));
		m_key.Draw();
	}
}
