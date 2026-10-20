#pragma once

/**
 * 調整用の値。Assets/param/ の JSON から読む
 * JSON のキーはメンバ名と同じつづりにする（JSON にはコメントが書けないので、意味と単位はここに書く）
 */

/** プレイヤーの値（player.json） */
struct PlayerParam
{
	/** 体 */
	struct Body
	{
		float chestHeight = 0.0f;		///< 足元から胸までの高さ。壁を探すレイ・ロープの手元・ロックの視線に使う
	};
	/** 地上・空中の移動 */
	struct Move
	{
		float speed = 0.0f;				///< 走る速さ
		float groundAccel = 0.0f;		///< 地上で加速する量（1秒あたり）
		float groundDecel = 0.0f;		///< 地上で減速する量（1秒あたり）
		float airControl = 0.0f;		///< 空中で、目標の速さに1秒あたりに近づく割合
	};
	/** ジャンプ */
	struct Jump
	{
		float speed = 0.0f;				///< 跳んだ瞬間の上向きの速さ
		int maxCount = 0;				///< 何回ジャンプできるか
	};
	/** ウォールラン */
	struct WallRun
	{
		float checkLength = 0.0f;		///< 壁を探すレイの長さ
		float normalYLimit = 0.0f;		///< 壁の向き（法線）の y の絶対値がこれより小さければ壁とみなす
		float speed = 0.0f;				///< 壁を走る速さ
		float minSpeed = 0.0f;			///< 壁走りを始めるのに必要な横の速さ
		float gravityScale = 0.0f;		///< 壁走り中の重力の倍率
		float stickSpeed = 0.0f;		///< 壁に押し付ける速さ
		float maxTime = 0.0f;			///< 落ちずに壁を走れる時間（秒）。過ぎるとゆっくりずり落ちる
		float jumpSpeedUp = 0.0f;		///< 壁ジャンプで上に跳ぶ速さ
		float jumpSpeedOut = 0.0f;		///< 壁ジャンプで壁から離れる速さ
		float reattachTime = 0.0f;		///< 壁を離れてから、もう一度張り付けるまでの時間（秒）
	};
	/** アンカーの自動ロック */
	struct Lock
	{
		float maxDistance = 0.0f;		///< ロックできる距離
		float maxAngle = 0.0f;			///< 画面の中央からの角度の上限（度）
		float angleWeight = 0.0f;		///< 角度の点の重み
		float distanceWeight = 0.0f;	///< 距離の点の重み
		float keepBonus = 0.0f;			///< 今ロックしているアンカーに足すおまけの点
	};
	/** グラップルのスイング */
	struct Swing
	{
		float wireShootSpeed = 0.0f;	///< ワイヤーの先が飛んでいく速さ（1秒あたり。3000 先まで約 0.4 秒）
		float startLift = 0.0f;			///< 地上でつかんだときに浮かせる上向きの速さ
		float pullRate = 0.0f;			///< ロープが伸びた分を、1秒あたりに戻す割合
		float reelSpeed = 0.0f;			///< 巻き上げでアンカーへ引き寄せる速さ（1秒あたり）
		float minRopeLength = 0.0f;		///< 巻き上げを止めるロープの長さ。ここからは振り子だけ
		float reelEase = 0.0f;			///< 最短の長さまでの残りに掛けて巻き上げの速さを決める（近いほど遅い）
		float pumpAccel = 0.0f;			///< スティックでこいだときの加速（1秒あたり）
		float faceMinSpeed = 0.0f;		///< スイング中に体の向きを変える、横の速さの下限
		float maxSpeed = 0.0f;			///< スイング中に出せる速さの上限
		float releaseBoost = 0.0f;		///< スイングを離したとき、進んでいる向きに足す速さ
		float releaseLift = 0.0f;		///< スイングを離したとき、上に足す速さ
		float releaseMinSpeed = 0.0f;	///< スイングを離したとき、これより遅いときはブーストしない
	};

	Body body;
	Move move;
	Jump jump;
	WallRun wallRun;
	Lock lock;
	Swing swing;
};

/** カメラの値（camera.json） */
struct CameraParam
{
	float distance = 0.0f;				///< 注視点からカメラまでの距離
	float targetHeight = 0.0f;			///< 足元から注視点までの高さ（胸から頭くらい）
	float rotateSpeedX = 0.0f;			///< 左右に回る速さ（度/秒）
	float rotateSpeedY = 0.0f;			///< 上下に回る速さ（度/秒）。左右より遅くすると扱いやすい
	float pitchMin = 0.0f;				///< 見下ろせる限界（度）
	float pitchMax = 0.0f;				///< 見上げられる限界（度）。上げすぎると地面にめり込む
	float viewAngleNormal = 0.0f;		///< 通常時の視野角（度）
	float viewAngleMax = 0.0f;			///< 最高速のときの視野角（度）
	float viewAngleLerpRate = 0.0f;		///< 目標の視野角へ近づく速さ（1秒あたり）
	float maxSpeedForFov = 0.0f;		///< この速さで視野角が最大になる
	float collisionRadius = 0.0f;		///< 壁めり込みを防ぐ球の半径（Start でだけ使うので、読み直しても変わらない）
	float nearClip = 0.0f;				///< 近くの描画の限界（Start でだけ使う）。near は Windows のマクロと重なるので使わない
	float farClip = 0.0f;				///< 遠くの描画の限界（Start でだけ使う。ステージが広いので大きめ）
};

/** 重力の値（gravity.json） */
struct GravityParam
{
	float gravity = 0.0f;				///< 重力の強さ（1秒あたりに増える落下速度）
	float fallMultiplier = 0.0f;		///< 落ちているときに重力に掛ける倍率
	float maxFallSpeed = 0.0f;			///< 落下速度の上限
};

/** アンカーの値（anchor.json） */
struct AnchorParam
{
	float normalScale = 0.0f;			///< ふだんの大きさ（1 で直径100）
	float lockedScale = 0.0f;			///< ロック中の大きさ
};

/** 銃と弾の値（gun.json） */
struct GunParam
{
	float shootInterval = 0.0f;			///< 射撃間隔
	float range = 0.0f;					///< 有効射程
	float bulletSpeed = 0.0f;			///< 弾丸の速さ
	float bulletLifeTime = 0.0f;		///< 弾丸が何秒残るか
	float bulletScale = 0.0f;			///< 弾丸の大きさ
	float aimHoldTime = 0.0f;			///< 撃つのをやめてから、体を前に向けておく時間・秒
};

/** グラップルのワイヤーの値（grappleWire.json） */
struct GrappleWireParam
{
	float thicknessScale = 0.0f;		///< 太さの倍率（1 で直径 2）
};

/** すべての調整値 */
struct GameParam
{
	PlayerParam player;
	CameraParam camera;
	GravityParam gravity;
	AnchorParam anchor;
	GrappleWireParam grappleWire;
	GunParam gun;
};

namespace nsGameParam
{
	/** すべての JSON を読む。ファイルやキーが足りなければ、どこが足りないかを表示してゲームを止める */
	void Load();
	/** 読み直す（開発中の調整用）。失敗したら表示して、前の値のまま続ける */
	void Reload();
	/** 読んだ値を返す。読み直しても同じ場所の中身が書き換わるので、参照を持っておいてよい */
	const GameParam& Get();
}
