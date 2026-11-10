#pragma once
#include "gimmick/Gimmick.h"
#include "gimmick/GimmickView.h"

/** 当たった瞬間に、基礎速度が減り、上乗せが 0 になり、はじかれてしびれる電気柵（罠）。Stage が持つ */
class ElectricFence : public Gimmick
{
public:
	void Draw(RenderContext& rc) override;

private:
	void InitView() override;
	void OnEnter(BattleCharacter& player) override;

private:
	static constexpr float DAMAGE = 100.0f;				///< 減らす基礎速度
	static constexpr float POST_INTERVAL = 600.0f;		///< 柱を立てる間隔（これより広くならないように、等間隔に立てる）
	static constexpr float WIRE_LENGTH = 100.0f;		///< gimmick_fence_wire.tkm の長さ

	GimmickView m_posts;								///< 見た目（柱）
	GimmickView m_wires;								///< 見た目（電線。幅に伸ばす）
};
