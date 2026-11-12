#pragma once
#include <vector>

/**
 * ギミックの見た目。同じモデルを何か所かに置いてまとめて描き、gimmick.fx で光らせる。ギミックが持つ
 * エンジンの ModelRender は専用シェーダーとインスタンシングを一緒に使えないので、Model を直接持つ
 */
class GimmickView : public IRenderer
{
public:
	/** 光り方（gimmick.fx の KIND_ と同じ値） */
	enum class Kind : int
	{
		DashPad = 0,	///< シアン。板の上を矢印が前へ流れる
		Fence = 1,		///< 黄色。電気がちらつき、火花が走る
		Laser = 2,		///< 赤。強く光る
	};

	/**
	 * @param tkmFilePath	モデルのファイル
	 * @param kind			光り方
	 * @param maxCount		置ける数
	 */
	void Init(const char* tkmFilePath, const Kind kind, const int maxCount);
	/** 1 つ置く。position はモデルの底の中心、scale は軸ごとの倍率 */
	void Add(const Vector3& position, const Vector3& scale);
	/** 描く予約をする（毎フレーム呼ぶ）。brightness は光る所の明るさの倍率（0 で光らない） */
	void Draw(const float brightness);

private:
	/** フォワードの描画のときに呼ばれる */
	void OnForwardRender(RenderContext& rc) override { m_model.Draw(rc, m_count); }

private:
	/** シェーダーに渡す値（gimmick.fx の Instance と同じ並び） */
	struct Instance
	{
		Vector4 position;		///< xyz が底の中心
		Vector4 scale;			///< xyz が倍率
	};

	/** シェーダーに渡す値（gimmick.fx の GimmickCb と同じ並び） */
	struct Constant
	{
		Vector3 cameraPos;		///< カメラの位置
		float time = 0.0f;		///< 経過時間（秒）
		float brightness = 1.0f;	///< 光る所の明るさの倍率
		float kind = 0.0f;		///< 光り方
		float padding[2] = {};	///< 16 バイトにそろえる
	};
	// cbuffer は 16 バイトずつ読むので、大きさを 16 の倍数にする（足りなければ padding を足す）
	static_assert(sizeof(Constant) % 16 == 0, "cbuffer size must be a multiple of 16 (add padding)");

	Model m_model;							///< モデル
	StructuredBuffer m_instanceBuffer;		///< m_instances をシェーダーへ送る
	std::vector<Instance> m_instances;		///< 置ける数の分の入れ物。使っているのは先頭の m_count 個
	Constant m_constant;					///< シェーダーに渡す値
	int m_maxCount = 0;						///< 置ける数
	int m_count = 0;						///< 置いた数（m_instances の大きさではなく、こちらで数える）
};
