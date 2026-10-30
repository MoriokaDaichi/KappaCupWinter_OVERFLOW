#include "stdafx.h"
#include "debug/DebugManager.h"

#include <iterator>
#include "actor/BattleCharacter.h"
#include "speed/SpeedSystem.h"

// ImGui に渡す日本語は U8() で包む（ソースの文字列は CP932 になるので、UTF-8 に直して渡す）

namespace
{
	constexpr int STATE_NUM = static_cast<int>(EnCharacterState::Num);	///< 状態の数（名前は CharacterStateId.h の STATE_NAMES）
}

void DebugManager::Init(BattleCharacter* player)
{
	m_player = player;
	// [this] で登録するが、m_panel はこのクラスと一緒に消えるので、消えた後に呼ばれることはない
	m_panel.Register("Game", "OVERFLOW", [this]() { DrawPanel(); }, true);
}

void DebugManager::Update()
{
	// F5 で JSON を読み直す（ゲームを止めずに値を調整するため）
	const bool isReloadKeyDown = (GetAsyncKeyState(VK_F5) & 0x8000) != 0;
	if (isReloadKeyDown && !m_isReloadKeyDown)
	{
		nsGameParam::Reload();
	}
	m_isReloadKeyDown = isReloadKeyDown;

	// Game はプレイヤーより先に Update されるので、ここで戻しておけば、このフレームの移動は固定した速度で行われる
	if (m_isSpeedLocked && m_player != nullptr)
	{
		m_player->GetSpeedSystem().SetForDebug(m_lockedBase, m_lockedBonus);
	}
}

void DebugManager::DrawPanel()
{
	if (m_player == nullptr)
	{
		return;
	}

	DrawSpeed();
	DrawPlayer();
	DrawWarp();
	DrawParam();
}

void DebugManager::DrawSpeed()
{
	SpeedSystem& speed = m_player->GetSpeedSystem();

	ImGui::SeparatorText(U8("速度"));
	ImGui::Text(U8("合計 %.0f（基礎 %.0f ＋ 上乗せ %.0f）"), speed.GetSpeed(), speed.GetBaseSpeed(), speed.GetBonusSpeed());

	char levelText[32];
	sprintf_s(levelText, "Lv%d  %.0f%%", speed.GetLevel(), speed.GetLevelRate() * 100.0f);
	ImGui::ProgressBar(speed.GetLevelRate(), ImVec2(-1.0f, 0.0f), levelText);

	// 固定しているときは固定した値を出す（そのまま出すと、地上で減った分だけスライダーが揺れる）
	float sliderBase = m_isSpeedLocked ? m_lockedBase : speed.GetBaseSpeed();
	float sliderBonus = m_isSpeedLocked ? m_lockedBonus : speed.GetBonusSpeed();
	bool isChanged = ImGui::SliderFloat(U8("基礎"), &sliderBase, m_speedParam.baseMin, m_speedParam.baseMax, "%.0f");
	isChanged |= ImGui::SliderFloat(U8("上乗せ"), &sliderBonus, 0.0f, m_speedParam.bonusMax, "%.0f");
	if (isChanged)
	{
		SetSpeed(sliderBase, sliderBonus);
	}

	// Lv の強制変更。その Lv の入口の速さにする
	const int levelNum = static_cast<int>(m_speedParam.levelUpSpeeds.size()) + 1;
	for (int level = 1; level <= levelNum; level++)
	{
		if (level > 1)
		{
			ImGui::SameLine();
		}

		char label[8];
		sprintf_s(label, "Lv%d", level);
		if (ImGui::Button(label))
		{
			// 基礎で足りない分は上乗せで足す（基礎の上限だけでは Lv5 に届かないため）
			const float target = (level == 1) ? m_speedParam.baseMin : m_speedParam.levelUpSpeeds[level - 2];
			const float base = min(target, m_speedParam.baseMax);
			// 地上では上乗せが減って Lv が戻ってしまうので、固定もする
			m_isSpeedLocked = true;
			SetSpeed(base, target - base);
		}
	}

	if (ImGui::Checkbox(U8("速度を固定する"), &m_isSpeedLocked) && m_isSpeedLocked)
	{
		SetSpeed(speed.GetBaseSpeed(), speed.GetBonusSpeed());
	}
}

void DebugManager::DrawPlayer()
{
	ImGui::SeparatorText(U8("プレイヤー"));

	const int state = static_cast<int>(m_player->GetCurrentState());
	ImGui::Text(U8("状態 %s"), (0 <= state && state < STATE_NUM) ? STATE_NAMES[state] : "-");

	// 0.0f を足すのは、-0 を 0 と表示するため
	const Vector3& position = m_player->GetPosition();
	ImGui::Text(U8("位置 (%.0f, %.0f, %.0f)"), position.x + 0.0f, position.y + 0.0f, position.z + 0.0f);

	Vector3 horizontal = m_player->GetMoveSpeed();
	const float vertical = horizontal.y;
	horizontal.y = 0.0f;
	ImGui::Text(U8("横の速さ %.0f　縦の速さ %.0f"), horizontal.Length(), vertical);
	ImGui::Text(U8("接地 %s"), m_player->IsOnGround() ? U8("している") : U8("していない"));
	if (m_player->HasGun())
	{
		ImGui::Text(U8("ロック %d 体"), m_player->GetGun().GetLockCount());
	}

	// debug.json にワープ先を足すときに、そのまま貼れる形でコピーする
	if (ImGui::Button(U8("今の位置をコピー")))
	{
		char text[128];
		sprintf_s(text, "{ \"name\": \"\", \"position\": [ %.1f, %.1f, %.1f ] }", position.x, position.y, position.z);
		ImGui::SetClipboardText(text);
	}
}

void DebugManager::DrawWarp()
{
	ImGui::SeparatorText(U8("ワープ"));

	for (int i = 0; i < static_cast<int>(m_param.warpPoints.size()); i++)
	{
		if (i > 0)
		{
			ImGui::SameLine();
		}

		const DebugParam::WarpPoint& point = m_param.warpPoints[i];
		// 名前が同じボタンでも区別できるように、番号を ID に足す
		ImGui::PushID(i);
		if (ImGui::Button(point.name.c_str()))
		{
			m_player->Warp(Vector3(point.position[0], point.position[1], point.position[2]));
		}
		ImGui::PopID();
	}
}

void DebugManager::DrawParam()
{
	ImGui::SeparatorText(U8("調整値"));

	if (ImGui::Button(U8("JSON を読み直す（F5）")))
	{
		nsGameParam::Reload();
	}
}

void DebugManager::SetSpeed(const float base, const float bonus)
{
	SpeedSystem& speed = m_player->GetSpeedSystem();
	speed.SetForDebug(base, bonus);

	// 範囲に収めた後の値を覚えておく
	m_lockedBase = speed.GetBaseSpeed();
	m_lockedBonus = speed.GetBonusSpeed();
}
