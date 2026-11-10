#pragma once
#include "gimmick/Gimmick.h"
#include "gimmick/GimmickView.h"

/**
 * ついたり消えたりするレーザーの壁（罠）。ついている間に当たると、基礎速度が減り、上乗せが 0 になり、はじかれてしびれる。Stage が持つ
 * 消えている間はビームが見えず、通り抜けられる。つく少し前から、ビームが弱く点滅して知らせる
 */
class LaserWall : public Gimmick
{
public:
	void Draw(RenderContext& rc) override;

private:
	void InitView() override;
	void OnEnter(BattleCharacter& player) override;
	void UpdateGimmick() override;
	bool IsActive() const override;

private:
	static constexpr float DAMAGE = 100.0f;				///< 減らす基礎速度
	static constexpr float ON_TIME = 2.0f;				///< ついている秒数
	static constexpr float OFF_TIME = 1.0f;				///< 消えている秒数
	static constexpr float WARNING_TIME = 0.5f;			///< つく何秒前から知らせるか
	static constexpr float WARNING_BLINK = 0.1f;		///< 知らせる点滅の 1 回分の秒数
	static constexpr float WARNING_BRIGHTNESS = 0.2f;	///< 知らせる点滅の明るさ
	static constexpr float POST_OFF_BRIGHTNESS = 0.2f;	///< 消えている間の、柱の発光口の明るさ
	static constexpr float POST_HALF_WIDTH = 50.0f;		///< gimmick_laser_post.tkm の幅の半分（壁にめりこまないように内側へ寄せる）
	static constexpr float BEAM_LENGTH = 100.0f;		///< gimmick_laser_beam.tkm の長さ

	float m_timer = 0.0f;								///< 点滅の 1 回分の中で、今何秒目か
	GimmickView m_posts;								///< 見た目（両端の柱）
	GimmickView m_beams;								///< 見た目（ビーム。幅に伸ばす）
};
