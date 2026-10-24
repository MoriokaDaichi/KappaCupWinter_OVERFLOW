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
	Vector3 aimOrigin = Vector3::Zero;		///< 狙いの始点（カメラの位置）
	Vector3 aimDirection = Vector3::Front;	///< 狙いの向き（カメラの前向き）
};
