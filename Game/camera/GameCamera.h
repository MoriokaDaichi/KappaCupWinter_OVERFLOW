#pragma once
#include "camera/CameraCollisionSolver.h"

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
	static constexpr float CAMERA_DISTANCE = 300.0f;		///< 注視点からカメラまでの距離
	static constexpr float TARGET_HEIGHT = 120.0f;			///< 足元から注視点までの高さ（胸から頭くらい）
	static constexpr float ROTATE_SPEED_X = 180.0f;			///< 左右に回る速さ（度/秒）
	static constexpr float ROTATE_SPEED_Y = 120.0f;			///< 上下に回る速さ（度/秒）。左右より遅くすると扱いやすい
	static constexpr float PITCH_MIN = -60.0f;				///< 見下ろせる限界（度）
	static constexpr float PITCH_MAX = 40.0f;				///< 見上げられる限界（度）。上げすぎると地面にめり込む
	static constexpr float VIEW_ANGLE_NORMAL = 60.0f;		///< 通常時の視野角（度）
	static constexpr float VIEW_ANGLE_MAX = 80.0f;			///< 最高速のときの視野角（度）
	static constexpr float VIEW_ANGLE_LERP_RATE = 5.0f;		///< 目標の視野角へ近づく速さ（1秒あたり）
	static constexpr float MAX_SPEED_FOR_FOV = 1500.0f;		///< この速さで視野角が最大になる
	static constexpr float COLLISION_RADIUS = 10.0f;			///< 壁めり込みを防ぐ球の半径
	static constexpr float CAMERA_NEAR = 1.0f;				///< 近くの描画の限界
	static constexpr float CAMERA_FAR = 10000.0f;			///< 遠くの描画の限界（ステージが広いので大きめ）

	CameraCollisionSolver m_collisionSolver;			///< カメラが壁にめり込まないように手前に寄せる
	const Player* m_player = nullptr;		///< 追いかける相手
	const InputManager* m_input = nullptr;	///< 右スティックを読むため

	Vector3 m_toCameraPos = Vector3::Zero;	///< 注視点からカメラまでのベクトル。スティックでこれを回す
	float m_viewAngle = 0.0f;				///< 今の視野角。目標の値に少しずつ近づけるので、値を持っておく
	float m_pitch = 0.0f;					///< 上下の角度（度）。プラスで見上げる。PITCH_MIN〜PITCH_MAX に収める
	float m_yaw = 0.0f;						///< 左右の角度（度）。0 でプレイヤーの後ろ（-Z 側）から見る
};
