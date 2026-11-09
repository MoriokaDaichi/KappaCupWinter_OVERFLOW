#include "stdafx.h"
#include "gimmick/GiantFirewall.h"

#include "actor/BattleCharacter.h"
#include "param/GameParam.h"
#include "effect/EffectManager.h"

void GiantFirewall::Init(BattleCharacter* player)
{
	m_player = player;

	const StageParam::Block& wall = nsGameParam::Get().stage.giantFirewall;

	const Vector3 position(wall.position[0], wall.position[1], wall.position[2]);
	const Vector3 size(wall.size[0], wall.size[1], wall.size[2]);

	m_collider.Create(size);
	
	RigidBodyInitData data;
	data.pos = position;
	data.collider = &m_collider;
	m_body.Init(data);

	// ライトに関係なく光らせるので、専用のシェーダーでフォワード描画する
	m_constant.center.Set(position.x, position.y, position.z, 0.0f);
	m_constant.halfSize.Set(size.x * 0.5f, size.y * 0.5f, size.z * 0.5f, 0.0f);
	m_textTexture.InitFromDDSFile(L"Assets/modelData/stage/giant_firewall_text.DDS");

	ModelInitData initData;
	initData.m_tkmFilePath = "Assets/modelData/debug/debug_wall.tkm";
	initData.m_fxFilePath = "Assets/shader/giantFirewall.fx";
	initData.m_expandShaderResoruceView[0] = &m_textTexture;
	initData.m_expandConstantBuffer = &m_constant;
	initData.m_expandConstantBufferSize = sizeof(m_constant);
	initData.m_colorBufferFormat[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
	m_model.InitForwardRendering(initData);
	m_model.SetPosition(position);
	m_model.SetScale(size.x / MODEL_SIZE, size.y / MODEL_SIZE, size.z / MODEL_SIZE);
	m_model.Update();

	m_flash.Init("Assets/modelData/debug/debug_white.DDS",
		static_cast<float>(g_graphicsEngine->GetFrameBufferWidth()),
		static_cast<float>(g_graphicsEngine->GetFrameBufferHeight()));
}

void GiantFirewall::Update()
{
	m_constant.cameraPos = g_camera3D->GetPosition();
	m_constant.time += g_gameTime->GetFrameDeltaTime();

	if (m_flashTimer > 0.0f)
	{
		m_flashTimer = max(m_flashTimer - g_gameTime->GetFrameDeltaTime(), 0.0f);
		m_flash.SetMulColor(Vector4(1.0f, 1.0f, 1.0f, m_flashTimer / FLASH_TIME));
		m_flash.Update();
	}

	if (m_isBroken)
	{
		return;
	}

	const StageParam::Block& wall = nsGameParam::Get().stage.giantFirewall;
	const Vector3& player = m_player->GetPosition();

	const float halfX = wall.size[0] * 0.5f;
	const float halfY = wall.size[1] * 0.5f;
	const float halfZ = wall.size[2] * 0.5f;
	const float front = wall.position[2] - halfZ;	// 手前の面（プレイヤーが来る側）
	const float back = wall.position[2] + halfZ;	// 奥の面

	const bool isTouchZ = player.z >= front - TOUCH_DISTANCE && player.z <= back;
	const bool isTouchX = fabsf(player.x - wall.position[0]) <= halfX;
	const bool isTouchY = player.y >= wall.position[1] - halfY && player.y <= wall.position[1] + halfY;

	if (!(isTouchZ && isTouchX && isTouchY))
	{
		return;
	}

	if (m_player->GetSpeedSystem().IsOverdrive())
	{
		Break();
	}
	else
	{
		Bounce();
	}
}

void GiantFirewall::Draw(RenderContext& rc)
{
	if (!m_isBroken)
	{
		m_model.Draw(rc);
	}

	if (m_flashTimer > 0.0f)
	{
		m_flash.Draw(rc);
	}
}

void GiantFirewall::Break()
{
	m_flashTimer = FLASH_TIME;

	m_body.Release();
	m_isBroken = true;

	const StageParam::Block& wall = nsGameParam::Get().stage.giantFirewall;
	const Vector3 chest = m_player->GetChestPosition();
	const Vector3 position(chest.x, chest.y, wall.position[2] - wall.size[2] * 0.5f);

	nsEffect::Play(EnEffect::Defeat, position, BREAK_EFFECT_SCALE);
}

void GiantFirewall::Bounce()
{
	m_player->ChangeState(EnCharacterState::Air);
	m_player->SetMoveSpeed(Vector3(0.0f, BOUNCE_UP_SPEED, -BOUNCE_BACK_SPEED));
}
