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
		float modelScale = 0.0f;		///< 見た目の倍率（砲身にも掛ける。Start で 1 回だけ使うので、F5 では変わらない）
		float hitRadius = 0.0f;			///< 弾の当たり判定のカプセルの半径（体がぶつかる判定とは別。敵は見た目より大きくして当てやすくする）
		float hitHeight = 0.0f;			///< 弾の当たり判定のカプセルの筒の部分の長さ（全体の高さは hitHeight + hitRadius × 2）
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
	/** 切り替え表の 1 行（JSON の transitions の 1 つ）。名前の一覧は actor/characterState/StateTransition.h */
	struct Transition
	{
		std::string from;						///< どの State のときか（"Any" ならどの State からでも）
		std::vector<std::string> when;			///< 条件の名前。全部成り立ったら切り替える
		std::string to;							///< 行き先の State
		std::vector<std::string> actions;		///< 切り替える前にやること（JSON のキーは "do"。書かなければなし）
	};
	/** アニメーション（書いたキャラだけ流す。名前の一覧は actor/CharacterAnimation.h） */
	struct Animation
	{
		/** 1 つのアニメーション */
		struct Clip
		{
			std::string anim;				///< どのアニメーションか（"Idle"・"Run"・"JumpUp"・"Fall"・"FallFast"・"Land"・"Swing"。全部 1 回ずつ書く）
			std::string file;				///< tka のファイル名
			bool loop = false;				///< 繰り返すか
		};
		std::vector<Clip> clips;				///< アニメーションの一覧
		float fastFallSpeed = 0.0f;			///< 落ちる速さがこれを超えたら、手足をばたつかせる落下アニメーションにする（重力の maxFallSpeed より小さくする）
		float landMinSpeed = 0.0f;			///< 着地したときの落ちる速さがこれを超えたら、着地のアニメーションを流す（小さな段差では流さない）
	};
	/** 手の骨の名前（書いたキャラだけ、ワイヤーを両手の間から出し、銃を右手に持たせる） */
	struct Hands
	{
		std::string left;						///< 左手
		std::string right;						///< 右手（銃の向きにも使う）
		std::string rightMiddle;				///< 右手の中指の付け根（銃を握る位置＝右手との間）
	};

	/** 体力（書いたキャラだけ弾が当たる。プレイヤーは速度が盾なので持たない） */
	struct Health
	{
		float maxHp = 0.0f;				///< 最初の体力。弾の damage の分だけ減り、0 以下で倒れる
		float killBoost = 0.0f;			///< 倒した相手の基礎速度に足す量（敵の種類で変える）
	};

	/** 台座の上に載せる砲身（書いたキャラだけ持つ）。長さは倍率 1 のときのモデルの大きさに合わせる（body.modelScale を掛けて使う） */
	struct Turret
	{
		std::string model;				///< 砲身のモデルのファイル名（.tkm）
		float height = 0.0f;			///< 足元から砲身のモデルの原点（底）までの高さ
		float pivotHeight = 0.0f;		///< 砲身の原点から回す中心までの高さ
		float muzzleHeight = 0.0f;		///< 砲身の原点から砲口までの高さ
		float muzzleForward = 0.0f;		///< 砲身の原点から砲口までの前への長さ
		float maxPitch = 0.0f;			///< 上下に向ける角度の上限（度）
	};

	std::string model;		///< モデルのファイル名（.tkm）
	std::vector<std::string> abilities;		///< 持つ能力の名前（"Grapple"・"Gun"）。書いていない能力は作らない
	std::vector<std::string> states;		///< 使う State の名前（"Idle"・"Run"・"Air"・"WallRun"・"Swing"）
	std::vector<Transition> transitions;	///< 切り替え表。上から順に調べ、最初に成り立った行で切り替える
	Body body;
	Move move;
	Jump jump;
	WallRun wallRun;
	Lock lock;
	Swing swing;
	Animation animation;
	bool hasAnimation = false;		///< JSON に animation を書いたか
	Hands hands;
	bool hasHands = false;			///< JSON に hands を書いたか
	Health health;
	bool hasHealth = false;			///< JSON に health を書いたか
	Turret turret;
	bool hasTurret = false;			///< JSON に turret を書いたか
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
	float lockAngle = 0.0f;				///< 画面の中央からこの角度（度）以内の敵をロックする
	float lockDistance = 0.0f;			///< この距離より遠い敵はロックしない
	float lockTime = 0.0f;				///< 照準に入れ続けてからロックするまでの時間（秒）
	float homingSpeed = 0.0f;			///< ホーミング弾の速さ
	float homingTurnRate = 0.0f;		///< ホーミング弾が 1 秒に相手の方へ曲がる割合
	float lockShootInterval = 0.0f;		///< ロック中に撃つ間隔（秒）
	float splitDelay = 0.0f;			///< ロック中に撃った弾が分裂するまでの時間（秒）

	struct Level
	{
		float shootInterval = 0.0f;			///< 射撃間隔（秒）
		float damage = 0.0f;				///< 弾 1 発のダメージ（爆発も同じ量）
		float bulletSpeed = 0.0f;			///< 弾丸の速さ
		float bulletScale = 0.0f;			///< 弾丸の大きさ
		bool pierce = false;				///< 弾が貫通するかどうか
		float explosionRadius = 0.0f;		///< 爆発の半径（0 なら爆発しない）
		int lockCount = 0;					///< 一度にロックできる数（0 ならマルチロックオンを使わない）
	};

	std::vector<Level> levels;				///< レベルの一覧
};

/** 敵の弾の値（enemy_bullet.json） */
struct EnemyBulletParam
{
	float speed = 0.0f;				///< 弾の速さ
	float lifeTime = 0.0f;			///< 弾が何秒残るか
	float scale = 0.0f;				///< 弾の大きさ
	float damage = 0.0f;			///< 当たったときに相手の基礎速度を減らす量
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

/** エフェクトの値（effect.json） */
struct EffectParam
{
	float hitScale = 0.0f;				///< 着弾のエフェクトの拡大率
	float defeatScale = 0.0f;			///< 撃破のエフェクトの拡大率
	float bulletScale = 0.0f;			///< 弾の光の筋の拡大率。gun.json の Lv ごとの bulletScale に掛ける
};

/** 敵を出す場所の値（spawn.json） */
struct SpawnParam
{
	/** 敵を出す場所 1 つ */
	struct Point
	{
		std::string chara;					///< どの敵か（"Walker"・"Firewall"・"Crawler"）
		std::array<float, 3> position = {};	///< 足元の位置（x, y, z）
		float respawnTime = 0.0f;			///< 倒されてから湧き直すまでの時間（秒）。0 なら湧き直さない
		float respawnRadius = 0.0f;			///< プレイヤーがここからこの距離以内にいる間だけ、湧き直しの時間が進む（敵が湧き直す区間）
	};

	std::vector<Point> points;				///< 敵を出す場所の一覧
};

/** ステージの値（stage.json） */
struct StageParam
{
	static constexpr int MAX_BLOCK = 256;			///< 箱の数の上限（インスタンシングの枠の数）
	static constexpr int MAX_TOWER = 128;			///< 遠景のビルの数の上限
	static constexpr int MAX_HAZARD = 32;			///< 穴の底の数の上限
	static constexpr int MAX_PROP = 512;			///< 小物の数の上限（種類ごと）
	static constexpr int PROP_TYPE_COUNT = 5;		///< 小物の種類の数

	/** 小物の種類 1 つ */
	struct PropType
	{
		const char* name;	///< JSON の model に書く名前。モデルは Assets/modelData/stage/stage_<name>.tkm
		float tint;			///< 色（0 でシアン、1 でマゼンタ）
		float colliderWidth;	///< 当たり判定の幅（倍率が 1 のとき）。0 なら当たり判定なし
		float colliderHeight;	///< 当たり判定の高さ
		float colliderDepth;	///< 当たり判定の奥行き
		float colliderCenterY;	///< 原点から当たり判定の中心までの高さ
	};

	/** 小物の種類の一覧 */
	static constexpr PropType PROP_TYPES[PROP_TYPE_COUNT] = {
		{ "gate_pillar", 0.0f, 200.0f, 2500.0f, 200.0f, 1250.0f },
		{ "gate_beam", 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },		// スイングが引っかかるので付けない
		{ "wall_pillar", 1.0f, 0.0f, 0.0f, 0.0f, 0.0f },	// 走りと壁走りが引っかかるので付けない
		{ "cable", 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },			// 同じ理由で付けない
		{ "machine", 1.0f, 700.0f, 500.0f, 700.0f, -250.0f },	// 下へ細くなる形なので、幅は中段に合わせる
	};

	/** 小物の種類の番号を返す。知らない名前なら -1 */
	static int FindPropType(const std::string& name)
	{
		for (int i = 0; i < PROP_TYPE_COUNT; i++)
		{
			if (name == PROP_TYPES[i].name)
			{
				return i;
			}
		}
		return -1;
	}

	/** 箱 1 つ */
	struct Block
	{
		std::array<float, 3> position = {};	///< 中心の位置（x, y, z）
		std::array<float, 3> size = {};		///< 幅・高さ・奥行き
	};

	/** 小物 1 つ（当たり判定の有無は PROP_TYPES で決まる） */
	struct Prop
	{
		std::string model;					///< 小物の種類（PROP_TYPES の name）
		std::array<float, 3> position = {};	///< 原点の位置（x, y, z）
		std::array<float, 3> scale = {};	///< 大きさの倍率
		float yaw = 0.0f;					///< Y 軸の回転（度）
	};

	/** 区間（①起動〜⑥突破）1 つ。区間ごとに分けるのは、JSON を読みやすくするため */
	struct Section
	{
		std::string name;							///< 区間の名前（エラーの表示に使う）
		std::vector<Block> blocks;					///< 箱の一覧
		std::vector<std::array<float, 3>> anchors;	///< アンカーの位置の一覧
		std::vector<Block> towers;					///< 遠景のビルの一覧（当たり判定なし）
		std::vector<Block> hazards;					///< 穴の底の一覧（当たり判定なし）
		std::vector<Prop> props;					///< 小物の一覧
	};

	std::array<float, 3> playerStart = {};	///< プレイヤーの足元の開始位置（Game::Start で 1 回だけ使う）
	std::vector<Section> sections;			///< 区間の一覧
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
	CharacterParam enemyFirewall;	///< 固定砲台 FIREWALL（chara/enemy_firewall.json）
	CharacterParam enemyCrawler;	///< 飛行型 CRAWLER（chara/enemy_crawler.json）
	SpawnParam spawn;
	StageParam stage;
	EffectParam effect;
	CameraParam camera;
	GravityParam gravity;
	AnchorParam anchor;
	GrappleWireParam grappleWire;
	GunParam gun;
	EnemyBulletParam enemyBullet;
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
	/** 読み直しに成功した回数を返す。値を読み直したことを知りたいクラス（Stage）が見る */
	int GetReloadCount();
	/** spawn.json の chara の名前から敵の設定を返す。読み直しても同じ場所を返す。知らない名前なら nullptr */
	const CharacterParam* FindEnemy(const std::string& name);
}
