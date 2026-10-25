#pragma once
#include "physics/Gravity.h"

/** プレイヤーと敵の共通部分（体・重力・モデル）。子クラスは UpdateAction だけを書く */
class Character : public IGameObject
{
public:
	/** UpdateAction → UpdateBody → OnBodyUpdated の順に呼ぶ。子クラスは上書きしない */
	void Update() override final;
	/** モデルの描画 */
	void Render(RenderContext& rc) override;

public:
	/** 地面にいるか */
	bool IsOnGround() const { return m_charaCon.IsOnGround(); }
	/** 下に向かっているか */
	bool IsFalling() const { return m_moveSpeed.y <= 0.0f; }
	/** 重力をかける。空中でだけ呼ぶ（床の上でかけると、毎フレーム床にめり込んでガタガタする） */
	void ApplyGravity();
	/** 横の速さを target に、1秒あたり rate だけ近づける */
	void ApproachHorizontal(const Vector3& target, float rate);
	/** 速度をわたす */
	const Vector3& GetMoveSpeed()const { return m_moveSpeed; }
	/** 上下の速さを決める（= で入れるので、落ちている途中でも同じ速さになる） */
	void SetVerticalSpeed(float speed) { m_moveSpeed.y = speed; }
	/** 速さを足す（跳ね返り・加速など） */
	void AddMoveSpeed(const Vector3& add) { m_moveSpeed += add; }
	/** 速さを決める（全部入れ替える） */
	void SetMoveSpeed(const Vector3& speed) { m_moveSpeed = speed; }
	/** 位置をわたす */
	const Vector3& GetPosition() const { return m_charaCon.GetPosition(); }
	/** dir の横の向き（y は見ない）に体を向ける。短すぎる向きのときは今の向きのまま */
	void FaceDirection(const Vector3& dir);
	/** 水平方向の速度を決める（ x と z だけを入れ替え、y はそのまま） */
	void SetHorizontalSpeed(const Vector3& speed);
	/** 重力の係数を決める */
	void SetGravityScale(float scale) { m_gravity.SetScale(scale); }

protected:
	/** 子クラスが書く（Player ならステートマシン） */
	virtual void UpdateAction() = 0;
	/** 体（位置・モデル・骨）を動かした後に呼ぶ。骨の位置を使う処理（手から出すワイヤーなど）はここに書く */
	virtual void OnBodyUpdated() {}
	/**
	 * モデルとキャラコンの初期化。子クラスの Start から呼ぶ
	 * clips はアニメーション（なければ nullptr と 0）。ModelRender は借りるだけなので、子クラスのメンバに持たせておく
	 * カプセルの高さは height + radius × 2（position は足元）
	 */
	void InitCharacter(const char* filePath, AnimationClip* clips, int clipCount, float radius, float height, const Vector3& position);

private:
	/** キャラクター共通（Execute → モデルに位置と向きを反映 → モデル更新） */
	void UpdateBody();

protected:
	CharacterController m_charaCon;	///< 当たり判定と移動
	Gravity m_gravity;				///< 重力
	Vector3 m_moveSpeed;			///< 移動速度
	ModelRender m_modelRender;		///< 見た目
	Quaternion m_rotation;			///< 向き
	Quaternion m_modelTilt = Quaternion::Identity;	///< 見た目だけの傾き。m_rotation の後にかける（当たり判定や向きの計算には使わない）
	float m_modelTiltPivotHeight = 0.0f;			///< 傾けるときの中心の、足元からの高さ

private:
	static constexpr float MIN_FACE_LENGTH_SQ = 0.001f;		///< 向きを決められる長さかどうかの境目（長さの2乗）
};
