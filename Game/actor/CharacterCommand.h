#pragma once

/** キャラクターがこのフレームで何をしたいかを書いておく */
struct CharacterCommand
{
	Vector3 moveDir = Vector3::Zero;		///< 進みたい方向（入力がなければ長さ 0）
	bool jumpTrigger = false;				///< ジャンプを押した瞬間
	bool grappleTrigger = false;			///< グラップルを押した瞬間
	bool grappleRelease = false;			///< グラップルを離した瞬間
	bool shootPress = false;				///< 撃っている間
	bool aimPress = false;					///< 構えている間
	Vector3 aimOrigin = Vector3::Zero;		///< 狙いの始点（プレイヤーはカメラの位置、敵は砲口）
	Vector3 aimDirection = Vector3::Front;	///< 狙いの向き（プレイヤーはカメラの前向き、敵は狙う点の方）
	float aimDistance = 0.0f;				///< 狙いの始点から狙う点までの距離（敵の予告線の長さに使う。0 なら決めていない）
};
