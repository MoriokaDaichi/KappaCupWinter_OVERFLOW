#pragma once
#include "stage/StageParticle.h"
#include "stage/StageProp.h"
#include "stage/StageRenderer.h"
#include "gimmick/Gimmick.h"
#include <array>
#include <memory>
#include <vector>

class Anchor;
class BattleCharacter;

/**
 * stage.json に書いた箱・アンカー・飾り（遠景のビル、穴の底、小物）を置く。Game が持つ
 * F5 で JSON を読み直すと作り直すので、ゲームを起動し直さずにステージを直せる
 */
class Stage
{
public:
	/** stage.json の箱とアンカーを作る */
	void Init();
	/** 毎フレーム Game の Update から呼ぶ。JSON を読み直していたら作り直す */
	void Update();
	/** アンカーを全部消す（Game が消えるときに呼ぶ） */
	void DeleteAll();
	/** 全部の箱を描く */
	void Draw(RenderContext& rc);
	/** ギミックが見るプレイヤーを受け取る（借りているだけ） */
	void SetPlayer(BattleCharacter* player) { m_player = player; }

private:
	/** 箱 1 つの当たり判定。物理エンジンが collider の場所を覚えるので、unique_ptr で持って場所を変えない */
	struct Block
	{
		BoxCollider collider;
		RigidBody body;
	};

	/** 今の stage.json の値で、箱とアンカーを置き直す */
	void Build();
	/** 当たり判定のある箱を、一覧の数だけ作る */
	void AddBlocks(const std::vector<StageParam::Block>& blocks, const StageRenderer::Kind kind);
	/** index 番のアンカーを position に置く。足りなければ作る */
	void PlaceAnchor(const int index, const Vector3& position);
	/** 当たり判定だけを作る */
	void AddCollider(const Vector3& position, const Vector3& size);
	/** 小物を 1 つ置く。種類によっては当たり判定も作る */
	void AddProp(const StageParam::Prop& prop);
	/** 当たり判定のない箱（遠景のビル、穴の底）を一覧の数だけ足す */
	void AddDecorations(const std::vector<StageParam::Block>& blocks, const StageRenderer::Kind kind);

private:
	static constexpr float HIDDEN_HEIGHT = -100000.0f;		///< 使わないアンカーを置く高さ（ロックの距離に入らない）

	std::vector<std::unique_ptr<Block>> m_blocks;			///< 全部の箱の当たり判定
	std::vector<Anchor*> m_anchors;							///< 全部のアンカー。読み直しても消さずに使い回す（スイング中のキャラが借りているため）
	std::vector<std::unique_ptr<Gimmick>> m_gimmicks;		///< 全部のギミック（ダッシュ板・電気柵・レーザー）。作り直すときは空にして作り直す
	StageRenderer m_renderer;								///< 全部の箱と背景の見た目
	std::array<StageProp, StageParam::PROP_TYPE_COUNT> m_props;	///< 小物の見た目（種類ごと。並びは StageParam::PROP_TYPES と同じ）
	StageParticle m_particle;								///< 舞うデータの粒
	int m_reloadCount = 0;									///< 最後に作ったときの、JSON を読み直した回数
	BattleCharacter* m_player = nullptr;					///< ギミックが見るプレイヤー
};
