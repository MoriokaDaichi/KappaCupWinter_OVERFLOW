#pragma once

/**
 * 画面を黒くする・黒から戻す（シーンの切り替えに使う）。main が 1 つだけ作り、ずっと残す
 * いちばん最後に描くように、優先度を大きくして作る（HUD やメニューの上に重ねるため）
 * 使う側は FindGO<Fade>(NAME) で探す
 */
class Fade : public IGameObject
{
public:
	bool Start() override;
	void Update() override;
	void Render(RenderContext& rc) override;

public:
	/** seconds 秒かけて黒くする */
	void StartFadeOut(const float seconds);
	/** seconds 秒かけて黒から戻す */
	void StartFadeIn(const float seconds);
	/** 黒くし終わったか（次のシーンを作ってよいか） */
	bool IsBlack() const { return m_alpha >= 1.0f && m_speed >= 0.0f; }

	static constexpr const char* NAME = "fade";		///< NewGO と FindGO で使う名前
	static constexpr int PRIORITY = 200;			///< 描く順番（大きいほど後に描く＝上に重なる）

private:
	static constexpr float MIN_SECONDS = 0.01f;		///< 秒数がこれより短いときは、この秒数にする（0 で割らないように）

	SpriteRender m_sprite;			///< 画面いっぱいの黒
	float m_alpha = 1.0f;			///< 今の濃さ（0 で見えない、1 で真っ黒）。最初は黒から始める
	float m_speed = 0.0f;			///< 1 秒あたりに濃さが変わる量（プラスで黒くなる、マイナスで戻る）
};
