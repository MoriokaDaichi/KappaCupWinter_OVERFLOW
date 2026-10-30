#pragma once

/** ロックオンのドーナツ 1 つ分。円の輪が、進み具合に合わせて上から時計回りに満ちていく（Assets/shader/lockOnRing.fx） */
class LockOnRing : public IRenderer
{
public:
	/** size は画面の上での直径（ピクセル） */
	void Init(const float size);
	/** screenPos は画面の中央を 0,0 とした位置。progress は塗る割合（0〜1） */
	void Update(const Vector2& screenPos, const float progress, const Vector4& color);
	/** 描く予約をする（毎フレーム呼ぶ。呼ばなかったフレームは出ない） */
	void Draw(RenderContext& rc);

private:
	/** 2D の描画のときに呼ばれる */
	void OnRender2D(RenderContext& rc) override { m_sprite.Draw(rc); }

private:
	/** シェーダーに渡す値（lockOnRing.fx の RingCb と同じ並び。16 バイトずつにそろえる） */
	struct RingData
	{
		Vector4 color;					///< 輪の色
		float progress = 0.0f;			///< 塗る割合（0〜1）
		float thickness = 0.0f;			///< 輪の太さ（半径に対する割合）
		float padding[2] = {};			///< 16 バイトにそろえるための空き
	};

	Sprite m_sprite;					///< 輪を描く板
	RingData m_ringData;				///< m_sprite がここを毎回シェーダーへ送る（Init の後は動かさない）

	static constexpr float RING_THICKNESS = 0.22f;	///< 輪の太さ（半径に対する割合）
};
