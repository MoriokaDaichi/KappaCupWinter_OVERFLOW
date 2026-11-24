#pragma once
#include "physics/Gravity.h"

/** プレイヤーと敵の共通部分（体・重力・モデル）。子クラスが UpdateAction を書く（戦うキャラは BattleCharacter が書く） */
class Character : public IGameObject
{
public:
	/** UpdateAction → UpdateBody → OnBodyUpdated の順に呼ぶ。ヒットストップ中は動かさず、見た目だけ震わせる。子クラスは上書きしない */
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
	void ApproachHorizontal(const Vector3& target, const float rate);
	/** 速度をわたす */
	const Vector3& GetMoveSpeed()const { return m_moveSpeed; }
	/** 上下の速さを決める（= で入れるので、落ちている途中でも同じ速さになる） */
	void SetVerticalSpeed(const float speed) { m_moveSpeed.y = speed; }
	/** 速さを足す（跳ね返り・加速など） */
	void AddMoveSpeed(const Vector3& add) { m_moveSpeed += add; }
	/** 速さを決める（全部入れ替える） */
	void SetMoveSpeed(const Vector3& speed) { m_moveSpeed = speed; }
	/** 位置をわたす */
	const Vector3& GetPosition() const { return m_charaCon.GetPosition(); }
	/** 体の向きを返す */
	const Quaternion& GetRotation() const { return m_rotation; }
	/** dir の横の向き（y は見ない）に体を向ける。短すぎる向きのときは今の向きのまま */
	void FaceDirection(const Vector3& dir);
	/** 水平方向の速度を決める（x と z だけを入れ替え、y はそのまま） */
	void SetHorizontalSpeed(const Vector3& speed);
	/** 重力の係数を決める */
	void SetGravityScale(const float scale) { m_gravity.SetScale(scale); }
	/** 渡された目標の傾きを覚えておく */
	void SetModelTiltTarget(const Quaternion& target, const float pivotHeight);
	/**
	 * ヒットストップを始める。seconds 秒のあいだ動きを止め、見た目だけを amplitude の幅で左右に震わせる（だんだん小さくする）
	 * 弾が当たった敵に使う（撃った側は止めない。走りの気持ちよさを止めないため）
	 */
	void StartHitStop(const float seconds, const float amplitude);

protected:
	/** 子クラスが書く（BattleCharacter なら、切り替え表と State を動かす） */
	virtual void UpdateAction() = 0;
	/** 体（位置・モデル・骨）を動かした後に呼ぶ。骨の位置を使う処理（手から出すワイヤーなど）はここに書く */
	virtual void OnBodyUpdated() {}
	/**
	 * モデルとキャラコンの初期化。子クラスの Start から呼ぶ
	 * アニメーションがなければ clips のない方を呼ぶ。AnimationClip は ModelRender が借りるだけなので、呼ぶ側が持っておく（BattleCharacter なら CharacterAnimation）
	 * カプセルの高さは height + radius × 2（position は足元）
	 */
	void InitCharacter(const char* filePath, std::vector<AnimationClip>& clips, const float radius, const float height, const Vector3& position);
	void InitCharacter(const char* filePath, const float radius, const float height, const Vector3& position);

private:
	/** キャラクター共通（Execute → モデルに位置と向きを反映 → モデル更新） */
	void UpdateBody();
	/** ヒットストップ中の 1 フレーム。時間を進め、止まった位置から左右に震わせる */
	void UpdateHitStop();

protected:
	CharacterController m_charaCon;	///< 当たり判定と移動
	Gravity m_gravity;				///< 重力
	Vector3 m_moveSpeed;			///< 移動速度
	ModelRender m_modelRender;		///< 見た目
	Quaternion m_rotation;			///< 向き
	Quaternion m_modelTilt = Quaternion::Identity;	///< 見た目だけの傾き。m_rotation の後にかける（当たり判定や向きの計算には使わない）
	Quaternion m_modelTiltTarget = Quaternion::Identity;		///< 目標の傾き。State が毎フレーム SetModelTiltTarget で決め、出さなければまっすぐに戻る
	float m_modelTiltPivotHeight = 0.0f;			///< 傾けるときの中心の、足元からの高さ
	Vector3 m_modelPosition;						///< 見た目の位置（傾きの分ずらした後）。UpdateBody で決める。輪郭線を体に重ねるのに使う
	Quaternion m_modelRotation = Quaternion::Identity;	///< 見た目の向き（向き × 傾き）。同上

private:
	static constexpr float MIN_FACE_LENGTH_SQ = 0.001f;		///< 向きを決められる長さかどうかの境目（長さの2乗）

	float m_hitStopTimer = 0.0f;			///< ヒットストップの残り（秒）。0 なら止まっていない
	float m_hitStopTime = 0.0f;				///< 今のヒットストップの長さ（秒）。震えを小さくしていく割合に使う
	float m_hitStopAmplitude = 0.0f;		///< 震えの幅
	Vector3 m_hitStopBasePosition;			///< 止まった瞬間の見た目の位置（ここを中心に震わせる）
	int m_hitStopFrame = 0;					///< 止まってからのフレーム数（偶数と奇数で左右を入れ替える）
};
