#pragma once

class BattleCharacter;

/** ステージの最後の巨大ファイアウォール。OVERDRIVE で触れたら壊れ、そうでなければ弾き返す。Game が持つ */
class GiantFirewall
{
public:
	/** 見張るプレイヤーを受け取り、壁を作る（Game の Start で呼ぶ） */
	void Init(BattleCharacter* player);
	/** 触れたかを調べて、壊すか弾き返す */
	void Update();
	/** 壊れていなければ壁を描く（Game の Render で呼ぶ） */
	void Draw(RenderContext& rc);
	/** 壊れたか */
	bool IsBroken() const { return m_isBroken; }
	/** 壊れる前に戻す（最初からやり直すとき） */
	void Reset();

private:
	/** 当たり判定を物理の世界に置く */
	void InitBody();
	/** 壊す */
	void Break();
	/** 弾き返す */
	void Bounce();

private:
	/** シェーダーに渡す値（giantFirewall.fx の FirewallCb と同じ並び） */
	struct Constant
	{
		Vector4 center;			///< 壁の中心
		Vector4 halfSize;		///< 壁の大きさの半分
		Vector3 cameraPos;		///< カメラの位置
		float time = 0.0f;		///< 経過時間（秒）
	};

	static constexpr float TOUCH_DISTANCE = 100.0f;			///< 壁の手前の面からこの距離まで近づいたら、触れたとみなす（当たり判定で手前に止まるため）
	static constexpr float BOUNCE_BACK_SPEED = 1200.0f;		///< 弾き返すときの、後ろ向きの速さ
	static constexpr float BOUNCE_UP_SPEED = 600.0f;		///< 弾き返すときの、上向きの速さ（空中に浮かせて、走る処理に上書きされないようにする）
	static constexpr float MODEL_SIZE = 100.0f;				///< 見た目に使うモデル（debug_wall.tkm）の 1 辺の長さ。大きさをこれで割って倍率にする
	static constexpr float BREAK_EFFECT_SCALE = 600.0f;		///< 突破のエフェクトの大きさ（敵の撃破は effect.json の defeatScale = 120。その 5 倍）
	static constexpr float FLASH_TIME = 0.5f;				///< 突破したとき、画面が白から透明に戻るまでの時間（秒）

	BattleCharacter* m_player = nullptr;					///< 見張るプレイヤー（借りているだけ）
	BoxCollider m_collider;									///< 当たり判定の形（箱）
	RigidBody m_body;										///< 物理の世界に置く体（Release で消せる）
	ModelRender m_model;									///< 見た目（赤い六角形の格子。Assets/shader/giantFirewall.fx）
	Texture m_textTexture;									///< 壁に出す文字「FIREWALL // ACCESS DENIED」
	Constant m_constant;									///< シェーダーに渡す値
	bool m_isBroken = false;								///< 壊れたか
	SpriteRender m_flash;									///< 突破したときに画面を白く光らせる絵
	float m_flashTimer = 0.0f;								///< 白い光が消えるまでの残り時間（秒）
};
