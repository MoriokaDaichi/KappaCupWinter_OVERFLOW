#pragma once
#include "param/GameParam.h"

class BattleCharacter;

/**
 * ステージのギミック（ダッシュ板・電気柵・レーザーの壁）の基底クラス
 * 箱の範囲にプレイヤーが入った瞬間に OnEnter を呼ぶ。何をするかと見た目は子クラスで決める。Stage が持つ
 */
class Gimmick
{
public:
	virtual ~Gimmick() = default;

	/** 位置と大きさを覚え、見た目を作る */
	void Init(const StageParam::Block& block);
	/** 入った瞬間なら OnEnter を呼ぶ（毎フレーム Stage から呼ぶ） */
	void Update(BattleCharacter& player);
	/** 見た目を描く（子クラスで書く） */
	virtual void Draw(RenderContext& rc) = 0;

protected:
	/** 見た目を作る（Init から呼ぶ。子クラスで書く） */
	virtual void InitView() = 0;
	/** プレイヤーが箱の範囲に入った瞬間にすること（子クラスで書く） */
	virtual void OnEnter(BattleCharacter& player) = 0;
	/** 毎フレームすること（Update の最初に呼ぶ。レーザーの点滅などで使う。なければ書かなくてよい） */
	virtual void UpdateGimmick() {}
	/** 今効くか。false の間は、中にいても「外」とみなす（レーザーが消えている間など） */
	virtual bool IsActive() const { return true; }
	/** 箱の範囲の中にいる間、毎フレームすること（なければ書かなくてよい） */
	virtual void OnStay(BattleCharacter& player) {}

protected:
	Vector3 m_position;					///< 箱の中心
	Vector3 m_size;						///< 箱の大きさ（幅・高さ・奥行き）

private:
	bool m_isPlayerInside = false;		///< 前のフレームに中にいたか（入った瞬間だけ OnEnter を呼ぶため）
};
