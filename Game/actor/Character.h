#pragma once
#include "physics/Gravity.h"

/** プレイヤーと敵の共通部分（体・重力・モデル）。子クラスは UpdateAction だけを書く */
class Character : public IGameObject
{
public:
	/** UpdateAction → UpdateBody の順に呼ぶ。子クラスは上書きしない */
	void Update() override final;
	/** モデルの描画 */
	void Render(RenderContext& rc) override;

public:
	/** 地面にいるか */
	bool IsOnGround() const { return m_charaCon.IsOnGround(); }
	/** 重力をかける。空中でだけ呼ぶ（床の上でかけると、毎フレーム床にめり込んでガタガタする） */
	void ApplyGravity();
	/** 速度をわたす */
	const Vector3& GetMoveSpeed()const { return m_moveSpeed; }
	/** 位置をわたす */
	const Vector3& GetPosition() const { return m_charaCon.GetPosition(); }

protected:
	/** 子クラスが書く（Player ならステートマシン） */
	virtual void UpdateAction() = 0;
	/**	モデルとキャラコンの初期化。子クラスの Start から呼ぶ */
	void InitCharacter(const char* filePath, float radius, float height, const Vector3& position);

private:
	/** キャラクター共通（Execute → モデルに位置と向きを反映 → モデル更新） */
	void UpdateBody();

protected:
	CharacterController m_charaCon;	///< 当たり判定と移動
	Gravity m_gravity;				///< 重力
	Vector3 m_moveSpeed;			///< 移動速度
	ModelRender m_modelRender;		///< 見た目
	Quaternion m_rotation;			///< 向き
};
