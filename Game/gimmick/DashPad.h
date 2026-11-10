#pragma once
#include "gimmick/Gimmick.h"
#include "gimmick/GimmickView.h"

/** 通ると上乗せが増えるダッシュ板。Stage が持つ */
class DashPad : public Gimmick
{
public:
	void Draw(RenderContext& rc) override;

private:
	void InitView() override;
	void OnEnter(BattleCharacter& player) override;
	void OnStay(BattleCharacter& player) override;

private:
	static constexpr float BOOST = 300.0f;					///< 入ったときに増やす上乗せ（上限600は SpeedSystem が守る）
	static constexpr float PUSH_SPEED = 600.0f;				///< 入ったときに、前（+Z）へ足す速さ
	static constexpr float MODEL_SIZE = 100.0f;				///< gimmick_dashpad.tkm の幅と奥行

	GimmickView m_view;		///< 見た目（矢印が流れる板）
};
