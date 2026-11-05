#pragma once

/**
 * カメラのまわりを舞うデータの粒を描く。位置はシェーダー（stageParticle.fx）の中で決める。Stage が持つ
 */
class StageParticle : public IRenderer
{
public:
	void Init();
	/** 描く予約をする（毎フレーム呼ぶ） */
	void Draw();

private:
	/** フォワードの描画のときに呼ばれる */
	void OnForwardRender(RenderContext& rc) override { m_model.Draw(rc, PARTICLE_COUNT); }

private:
	/** シェーダーに渡す値（stageParticle.fx の ParticleCb と同じ並び） */
	struct Constant
	{
		Vector3 cameraPos;		///< カメラの位置
		float time = 0.0f;		///< 経過時間（秒）
	};

	static constexpr int PARTICLE_COUNT = 400;	///< 粒の数

	Model m_model;			///< 板として使う箱のモデル
	Constant m_constant;	///< シェーダーに渡す値
};
