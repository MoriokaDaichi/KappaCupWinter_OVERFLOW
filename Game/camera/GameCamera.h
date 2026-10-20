#pragma once
#include "camera/CameraCollisionSolver.h"
#include "param/GameParam.h"

class Player;
class InputManager;

/** プレイヤーに追従するばねカメラ。プレイヤーの速度によって視野角がかわる */
class GameCamera : public IGameObject
{
public:
	bool Start() override;
	void Update() override;

public:
	/** 追いかける相手を受け取る */
	void SetTarget(const Player* player) { m_player = player; }
	/** 入力を受け取る */
	void SetInput(const InputManager* input) { m_input = input; }

private:
	/** スティックでカメラの向きを回す */
	void Rotate();
	/** 速さに合わせて視野角を変える */
	void UpdateViewAngle();
	/** 注視点と位置を計算してカメラに渡す */
	void UpdatePosition();

private:
	const CameraParam& m_param = nsGameParam::Get().camera;	///< 調整用の値（Assets/param/camera.json）

	CameraCollisionSolver m_collisionSolver;			///< カメラが壁にめり込まないように手前に寄せる
	const Player* m_player = nullptr;		///< 追いかける相手
	const InputManager* m_input = nullptr;	///< 右スティックを読むため

	Vector3 m_toCameraPos = Vector3::Zero;	///< 注視点からカメラまでのベクトル。スティックでこれを回す
	float m_viewAngle = 0.0f;				///< 今の視野角。目標の値に少しずつ近づけるので、値を持っておく
	float m_pitch = 0.0f;					///< 上下の角度（度）。プラスで見上げる。pitchMin〜pitchMax に収める
	float m_yaw = 0.0f;						///< 左右の角度（度）。0 でプレイヤーの後ろ（-Z 側）から見る
};
