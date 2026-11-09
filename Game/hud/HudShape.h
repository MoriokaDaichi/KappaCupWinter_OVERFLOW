#pragma once

/**
 * HUD の形 1 つ分（スピードメーター・照準・角カッコなど）。形はシェーダーが描くので、画像はいらない
 * シェーダーには色と値（0〜1 の割合など）と時間を渡す。どの値を使うかはシェーダーごとに決める
 */
class HudShape : public IRenderer
{
public:
	/**
	 * @param fxFilePath	形を描くシェーダー（Assets/shader/hud*.fx。ShapeCb を同じ並びで受け取る）
	 * @param width			画面の上での幅（ピクセル）
	 * @param height		画面の上での高さ（ピクセル）
	 */
	void Init(const char* fxFilePath, const float width, const float height);
	/** screenPos は画面の中央を 0,0 とした中心の位置。value はシェーダーに渡す値 */
	void Update(const Vector2& screenPos, const Vector4& color, const float value, const float flag = 0.0f);
	/** 描く予約をする（毎フレーム呼ぶ。呼ばなかったフレームは出ない） */
	void Draw();

private:
	/** 2D の描画のときに呼ばれる */
	void OnRender2D(RenderContext& rc) override { m_sprite.Draw(rc); }

private:
	/** シェーダーに渡す値（hud*.fx の ShapeCb と同じ並び。16 バイトずつにそろえる） */
	struct ShapeData
	{
		Vector4 color;					///< 色
		float value = 0.0f;				///< 値（スピードメーターなら針の割合）
		float time = 0.0f;				///< 経過時間（秒）。光の明滅に使う
		float flag = 0.0f;				///< 切り替え（スピードメーターなら OVERDRIVE 中で 1）
		float padding = 0.0f;			///< 16 バイトにそろえるための空き
	};

	Sprite m_sprite;					///< 形を描く板
	ShapeData m_shapeData;				///< m_sprite がここを毎回シェーダーへ送る（Init の後は動かさない）
};
