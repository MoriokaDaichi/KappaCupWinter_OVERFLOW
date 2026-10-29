#pragma once
#include <array>
#include <string>
#include <vector>

/**
 * 調整用の値。Assets/param/ の JSON から読む
 * JSON のキーはメンバ名と同じつづりにする（JSON にはコメントが書けないので、意味と単位はここに書く）
 */

/** キャラクターの値（Assets/param/chara/ の JSON。プレイヤーも敵も同じ形） */
struct CharacterParam
{
	/** 体 */
	struct Body
	{
		float radius = 0.0f;			///< 当たり判定のカプセルの半径
		float height = 0.0f;			///< カプセルの筒の部分の長さ（全体の高さは height + radius × 2。Medea の身長 165 に合わせる）
		float chestHeight = 0.0f;		///< 足元から胸までの高さ。壁を探すレイ・ロープの手元・ロックの視線に使う
		float tiltLerpRate = 0.0f;		///< 体の見た目の傾き（スイング・壁走り）を目標に近づける速さ。1秒あたりの割合
	};
	/** 地上・空中の移動 */
	struct Move
	{
		float groundAccel = 0.0f;		///< 地上で加速する量（1秒あたり）
		float groundDecel = 0.0f;		///< 地上で減速する量（1秒あたり）
		float airControl = 0.0f;		///< 空中で、目標の速さに1秒あたりに近づく割合
	};
	/** ジャンプ */
	struct Jump
	{
		float speed = 0.0f;				///< 跳んだ瞬間の上向きの速さ
		int maxCount = 0;				///< 何回ジャンプできるか
		float fastFallAnimSpeed = 0.0f;	///< 落ちる速さがこれを超えたら、手足をばたつかせる落下アニメーションにする（重力の maxFallSpeed より小さくする）
		float landAnimMinSpeed = 0.0f;	///< 着地したときの落ちる速さがこれを超えたら、着地のアニメーションを流す（小さな段差では流さない）
	};
	/** ウォールラン */
	struct WallRun
	{
		float checkLength = 0.0f;		///< 壁を探すレイの長さ
		float normalYLimit = 0.0f;		///< 壁の向き（法線）の y の絶対値がこれより小さければ壁とみなす
		float speed = 0.0f;				///< 壁を走る最低の速さ（速度システムの値のほうが速ければ、そちらで走る）
		float minSpeed = 0.0f;			///< 壁走りを始めるのに必要な横の速さ
		float gravityScale = 0.0f;		///< 壁走り中の重力の倍率
		float stickSpeed = 0.0f;		///< 壁に押し付ける速さ
		float maxTime = 0.0f;			///< 落ちずに壁を走れる時間（秒）。過ぎるとゆっくりずり落ちる
		float jumpSpeedUp = 0.0f;		///< 壁ジャンプで上に跳ぶ速さ
		float jumpSpeedOut = 0.0f;		///< 壁ジャンプで壁から離れる速さ
		float reattachTime = 0.0f;		///< 壁を離れてから、もう一度張り付けるまでの時間（秒）
		float tiltAngle = 0.0f;			///< 壁走り中に、体を壁から離す向きへ傾ける角度（度）。足が壁につく高さを中心に回す
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

	std::string model;		///< モデルのファイル名（.tkm）
	std::vector<std::string> abilities;		///< 持つ能力の名前（"Grapple"・"Gun"）。書いていない能力は作らない
	std::vector<std::string> states;		///< 使う State の名前（"Idle"・"Run"・"Air"・"WallRun"・"Swing"）
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
	float shoulderOffset = 0.0f;		///< 注視点を右にずらす量
	float aimDistance = 0.0f;			///< 構えたときの、注視点からカメラまでの距離
	float aimShoulderOffset = 0.0f;		///< 構えたときの、肩のずらし量
	float aimViewAngleDown = 0.0f;		///< 構えたときに視野角を狭める量（度）
	float aimRotateScale = 0.0f;		///< 構えたときの感度の倍率
	float aimTransitionTime = 0.0f;		///< 構える／戻すのにかかる時間（秒）
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
	float range = 0.0f;					///< 有効射程
	float bulletLifeTime = 0.0f;		///< 弾丸が何秒残るか
	float aimHoldTime = 0.0f;			///< 撃つのをやめてから、体を前に向けておく時間・秒
	float muzzleRight = 0.0f;			///< 銃口の位置。胸から体の右へずらす量
	float muzzleForward = 0.0f;			///< 銃口の位置。胸から体の前へずらす量

	struct Level
	{
		float shootInterval = 0.0f;			///< 射撃間隔（秒）
		float bulletSpeed = 0.0f;			///< 弾丸の速さ
		float bulletScale = 0.0f;			///< 弾丸の大きさ
		bool pierce = false;				///< 弾が貫通するかどうか
		float explosionRadius = 0.0f;		///< 爆発の半径（0 なら爆発しない）
	};

	std::vector<Level> levels;				///< レベルの一覧
};

/** グラップルのワイヤーの値（grappleWire.json） */
struct GrappleWireParam
{
	float thicknessScale = 0.0f;		///< 太さの倍率（1 で直径 2）
};

/** 速度システムの値（speed.json） */
struct SpeedParam
{
	float baseMin = 0.0f;				///< 基礎速度の下限
	float baseMax = 0.0f;				///< 基礎速度の上限
	float bonusMax = 0.0f;				///< 上乗せの上限
	float groundDecay = 0.0f;			///< 地上にいるときに、上乗せが1秒あたりに減る量
	float idleDecay = 0.0f;				///< 止まっているときに、上乗せが1秒あたりに減る量
	float wallRunGain = 0.0f;			///< 壁走り中に、上乗せが1秒あたりに増える量
	std::array<float, 4> levelUpSpeeds = {};	///< Lv2・3・4・5 になる速さ
	float maxSpeed = 0.0f;				///< メーターが振り切る速さ
};

/** デバッグ画面の値（debug.json） */
struct DebugParam
{
	/** ワープ先 */
	struct WarpPoint
	{
		std::string name;					///< ボタンに出す名前
		std::array<float, 3> position = {};	///< 足元の位置（x, y, z）
	};

	std::vector<WarpPoint> warpPoints;		///< ワープ先の一覧（チェックポイントができたら置き換える）
};

/** すべての調整値 */
struct GameParam
{
	CharacterParam player;			///< プレイヤー（chara/player.json）
	CharacterParam enemyWalker;		///< 歩く敵（chara/enemy_walker.json）
	CameraParam camera;
	GravityParam gravity;
	AnchorParam anchor;
	GrappleWireParam grappleWire;
	GunParam gun;
	SpeedParam speed;
	DebugParam debug;
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
