#include "stdafx.h"
#include "gimmick/LaserWall.h"

#include "actor/BattleCharacter.h"
#include "sound/GameSound.h"

void LaserWall::Draw(RenderContext& rc)
{
	if (IsActive())
	{
		m_posts.Draw(1.0f);
		m_beams.Draw(1.0f);
		return;
	}

	m_posts.Draw(POST_OFF_BRIGHTNESS);

	// つく少し前から、ビームを弱く点滅させて知らせる
	const bool isWarning = m_timer >= ON_TIME + OFF_TIME - WARNING_TIME;
	const bool isBlinkOn = std::fmod(m_timer, WARNING_BLINK) < WARNING_BLINK * 0.5f;
	if (isWarning && isBlinkOn)
	{
		m_beams.Draw(WARNING_BRIGHTNESS);
	}
}

void LaserWall::InitView()
{
	const float bottom = m_position.y - m_size.y * 0.5f;
	const float halfWidth = m_size.x * 0.5f - POST_HALF_WIDTH;

	m_posts.Init("Assets/modelData/gimmick/gimmick_laser_post.tkm", GimmickView::Kind::Laser, 2);
	m_posts.Add(Vector3(m_position.x - halfWidth, bottom, m_position.z), Vector3::One);
	m_posts.Add(Vector3(m_position.x + halfWidth, bottom, m_position.z), Vector3::One);

	m_beams.Init("Assets/modelData/gimmick/gimmick_laser_beam.tkm", GimmickView::Kind::Laser, 1);
	m_beams.Add(Vector3(m_position.x, bottom, m_position.z), Vector3(m_size.x / BEAM_LENGTH, 1.0f, 1.0f));
}

void LaserWall::OnEnter(BattleCharacter& player)
{
	nsSound::Play(EnSound::LaserHit);
	HitTrap(player, DAMAGE);
}

void LaserWall::UpdateGimmick()
{
	m_timer += g_gameTime->GetFrameDeltaTime();

	// 1 回分（つく＋消える）を過ぎたら、はみ出した分を残して最初に戻す
	const float cycle = ON_TIME + OFF_TIME;
	if (m_timer >= cycle)
	{
		m_timer -= cycle;
	}
}

bool LaserWall::IsActive() const
{
	return m_timer < ON_TIME;
}
