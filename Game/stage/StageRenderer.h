#pragma once
#include "param/GameParam.h"
#include <array>

/**
 * ステージの箱・遠景のビル・穴の底と背景を、ネオンの見た目（Assets/shader/stageBlock.fx）で描く。Stage が持つ
 * エンジンの ModelRender は専用シェーダーとインスタンシングを一緒に使えないので、Model を直接持つ
 */
class StageRenderer : public IRenderer
{
public:
	/** 描く物の種類（stageBlock.fx の KIND_ と同じ番号） */
	enum class Kind : int
	{
		Block,		///< 当たり判定のある箱
		Tower,		///< 遠景のビル
		Hazard,		///< 穴の底
		Panel,		///< 壁走りの板（当たり判定あり）
	};

	void Init();
	/** 箱を全部なくす（置き直す前に呼ぶ） */
	void ClearBlocks() { m_instanceCount = FIRST_BLOCK; }
	/** 箱を 1 つ足す */
	void AddBlock(const Vector3& position, const Vector3& size, const Kind kind = Kind::Block);
	/** 描く予約をする（毎フレーム呼ぶ） */
	void Draw();

private:
	/** フォワードの描画のときに呼ばれる */
	void OnForwardRender(RenderContext& rc) override { m_model.Draw(rc, m_instanceCount); }

private:
	/** シェーダーに渡す値（stageBlock.fx の Instance と同じ並び） */
	struct Instance
	{
		Vector4 center;		///< 中心の位置
		Vector4 halfSize;	///< xyz が大きさの半分。w が種類（Kind）
	};

	/** シェーダーに渡す値（stageBlock.fx の StageCb と同じ並び） */
	struct Constant
	{
		float time = 0.0f;	///< 経過時間（秒）
		float padding[3] = {};	///< 16 バイトにそろえるための空き
	};
	// cbuffer は 16 バイトずつ読むので、大きさを 16 の倍数にする（足りなければ padding を足す）
	static_assert(sizeof(Constant) % 16 == 0, "cbuffer size must be a multiple of 16 (add padding)");

	static constexpr int FIRST_BLOCK = 1;										///< 0 番は背景なので、箱は 1 番から入れる
	static constexpr int MAX_INSTANCE = StageParam::MAX_BLOCK + StageParam::MAX_TOWER + StageParam::MAX_HAZARD + FIRST_BLOCK;	///< 枠の数
	static constexpr float EXPOSURE_SCALE = 0.3f;								///< 自動露出の目標の明るさに掛ける値
	static constexpr float LIGHT_BRIGHTNESS = 5.0f;								///< キャラに当たるライトの明るさ
	static constexpr float LIGHT_DIRECTION_X = 0.4f;							///< ライトの向き
	static constexpr float LIGHT_DIRECTION_Y = -1.0f;
	static constexpr float LIGHT_DIRECTION_Z = 0.7f;
	static constexpr float BACKGROUND_SIZE = 1000.0f;							///< 背景の箱の大きさの半分（いちばん奥の深度で描くので、カメラを囲めばよい）

	Model m_model;										///< 箱のモデル
	Texture m_wallMask;									///< 壁の模様（R=回路 G=パネル B=データ）
	StructuredBuffer m_instanceBuffer;					///< m_instances をシェーダーへ送る
	std::array<Instance, MAX_INSTANCE> m_instances;		///< 全部の箱
	int m_instanceCount = FIRST_BLOCK;					///< 使っている枠の数
	Constant m_constant;								///< シェーダーに渡す値
};
