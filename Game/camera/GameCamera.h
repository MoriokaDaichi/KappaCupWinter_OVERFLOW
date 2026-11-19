#pragma once
#include "camera/CameraCollisionSolver.h"
#include "param/GameParam.h"

class BattleCharacter;
class InputManager;

/** プレイヤーに追従するばねカメラ。プレイヤーの速度によって後ろへ引く・視野角がかわる・スイング中は高さを遅れて追う・壁走りで傾ける */
class GameCamera : public IGameObject
{
public:
	bool Start() override;
	void Update() override;

public:
	/** 追いかける相手を受け取る */
	void SetTarget(const BattleCharacter* player) { m_player = player; }
	/** 入力を受け取る */
	void SetInput(const InputManager* input) { m_input = input; }

private:
	/** スティックでカメラの向きを回す */
	void Rotate();
	/** 速さの割合を出し（引く距離にも使う）、視野角を決める。速度 Lv が上がった瞬間は、一瞬広げて戻す */
	void UpdateSpeedRate();
	/** 注視点と位置を計算してカメラに渡す */
	void UpdatePosition();
	/** 壁走り中は壁から離れる側へ傾け、それ以外はまっすぐに戻す */
	void UpdateRoll();
	/** 壁走り中の壁が、画面の左右どちらにあるかを調べて m_wallSide に入れる（傾きと肩で使う） */
	void UpdateWallSide();
	/** カメラの右の向き（水平。m_yaw で回したもの）を返す */
	Vector3 GetCameraRight() const;

private:
	const CameraParam& m_param = nsGameParam::Get().camera;	///< 調整用の値（Assets/param/camera.json）

	CameraCollisionSolver m_collisionSolver;			///< カメラが壁にめり込まないように手前に寄せる
	const BattleCharacter* m_player = nullptr;		///< 追いかける相手
	const InputManager* m_input = nullptr;	///< 右スティックを読むため

	static constexpr float SHOULDER_WALL_MARGIN = 5.0f;		///< 肩の点が壁に当たったとき、体の中心の方へ戻す量
	static constexpr float CAMERA_MIN_DISTANCE = 1.0f;		///< カメラと注視点がこれより近いときは、この距離だけ離す

	Vector3 m_toCameraPos = Vector3::Zero;	///< 注視点からカメラまでのベクトル。スティックでこれを回す
	float m_speedRate = 0.0f;				///< 速さの割合（0〜1）。少しずつ近づける。視野角と引く距離に使う
	float m_pitch = 0.0f;					///< 上下の角度（度）。プラスで見上げる。pitchMin〜pitchMax に収める
	float m_yaw = 0.0f;						///< 左右の角度（度）。0 でプレイヤーの後ろ（-Z 側）から見る
	float m_kickTimer = 0.0f;				///< Lv アップで広げた視野角が戻るまでの残り時間（秒）
	int m_lastLevel = 0;					///< 前のフレームの速度 Lv（上がった瞬間を知るため）
	float m_pivotY = 0.0f;					///< 注視点の高さ。プレイヤーの高さを少し遅れて追う
	int m_lastWarpCount = 0;				///< 前のフレームで見たワープの回数
	float m_roll = 0.0f;					///< 今の傾き（度）。プラスで右へ傾く
	int m_wallSide = 0;						///< 壁走り中の壁の側（画面の右 +1、左 -1、壁走りでないか壁がなければ 0）
};
