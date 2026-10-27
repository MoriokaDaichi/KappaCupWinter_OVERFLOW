#pragma once
#include "param/GameParam.h"

/** グラップルのワイヤー。始点と終点を受け取って、細長いモデルを伸ばして描く */
class GrappleWire : public IGameObject
{
public:
	bool Start() override;
	void Update() override;
	void Render(RenderContext& rc) override;

public:
	/** 表示するかどうかを決める。スイングを始めたら true、終わったら false */
	void SetVisible(const bool visible) { m_isVisible = visible; }
	/** 狙う先と、先が飛んだ長さを覚える */
	void SetTarget(const Vector3& anchorPosition, const float tipLength);
	/** Playerの手元を覚える */
	void SetHandPosition(const Vector3& hand);

private:
	/** 両端から、モデルの位置・向き・大きさを決める */
	void UpdateTransform();

private:
	static constexpr float MODEL_LENGTH = 100.0f;		///< モデルの元の長さ（Z 方向。原点は片方の端）
	static constexpr float MIN_LENGTH = 0.001f;			///< これより短いと向きを出せないので描かない

	const GrappleWireParam& m_param = nsGameParam::Get().grappleWire;	///< 調整用の値（Assets/param/grappleWire.json）

	ModelRender m_modelRender;					///< 見た目
	Vector3 m_start = Vector3::Zero;			///< 始点（プレイヤーの手元）
	Vector3 m_targetPosition = Vector3::Zero;	///< 狙う先（アンカー）
	float m_tipLength = 0.0f;					///< 手元から飛んだ先までの長さ
	bool m_isVisible = false;					///< 表示するか
	bool m_canDraw = false;						///< 今のフレームで描けるか（短すぎるときは false）
};
