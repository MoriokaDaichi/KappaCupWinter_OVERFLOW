#pragma once
#include "param/GameParam.h"
#include <vector>

/**
 * 同じ形の小物（ゲート・柱など）をまとめて描く。当たり判定は Stage が作る。Stage が種類ごとに持つ
 * エンジンの ModelRender は専用シェーダーとインスタンシングを一緒に使えないので、Model を直接持つ
 */
class StageProp : public IRenderer
{
public:
	/**
	 * @param tkmFilePath	モデルのファイル
	 * @param tint			色（0 でシアン、1 でマゼンタ）
	 */
	void Init(const char* tkmFilePath, const float tint);
	/** 置いた小物を全部なくす（置き直す前に呼ぶ） */
	void Clear() { m_instances.clear(); }
	/** 小物を 1 つ足す */
	void Add(const StageParam::Prop& prop);
	/** 描く予約をする（毎フレーム呼ぶ） */
	void Draw();

private:
	/** フォワードの描画のときに呼ばれる */
	void OnForwardRender(RenderContext& rc) override { m_model.Draw(rc, static_cast<int>(m_instances.size())); }

private:
	/** シェーダーに渡す値（stageProp.fx の Instance と同じ並び） */
	struct Instance
	{
		Vector4 positionYaw;	///< xyz が位置。w が Y 軸の回転（ラジアン）
		Vector4 scaleTint;		///< xyz が大きさの倍率。w が色
	};

	/** シェーダーに渡す値（stageProp.fx の PropCb と同じ並び） */
	struct Constant
	{
		Vector3 cameraPos;		///< カメラの位置
		float time = 0.0f;		///< 経過時間（秒）
	};

	Model m_model;							///< 小物のモデル
	StructuredBuffer m_instanceBuffer;		///< m_instances をシェーダーへ送る
	std::vector<Instance> m_instances;		///< 置いた小物
	Constant m_constant;					///< シェーダーに渡す値
	float m_tint = 0.0f;					///< 色
};
