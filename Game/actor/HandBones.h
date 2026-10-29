#pragma once
#include "param/GameParam.h"

/**
 * 手の骨の場所を調べる係（もとは Player の中にあったもの）
 * ワイヤーを出す両手の間と、銃を握る右手の位置・向きを、骨が動いた後に返す
 * JSON に hands を書いたキャラだけが持つ（BattleCharacter が作る）
 */
class HandBones
{
public:
	/** 骨を名前で探しておく（InitCharacter の後に呼ぶ）。見つからない骨は使わない */
	void Init(const ModelRender& model, const CharacterParam::Hands& hands);
	/** 両手の間の位置を outPosition に入れる（ロープの手元）。左右どちらかの骨がなければ false */
	bool GetRopeHandPosition(Vector3& outPosition) const;
	/** 右手の骨の行列（銃の向き）と、握る位置（手首と中指の付け根の間＝手のひら）を入れる。骨がなければ false */
	bool GetGrip(Matrix& outHandMatrix, Vector3& outGripPosition) const;

private:
	/** 骨の名前から番号を探す。見つからなければ -1 */
	int FindBone(const std::string& name) const;
	/** 2 つの骨の位置のまん中 */
	Vector3 GetMiddle(const int boneA, const int boneB) const;

private:
	const ModelRender* m_model = nullptr;	///< 体のモデル（借りているだけ）
	int m_leftHandBoneId = -1;				///< 左手の骨の番号（ロープの手元に使う。見つからなければ -1）
	int m_rightHandBoneId = -1;				///< 右手の骨の番号
	int m_rightMiddleBoneId = -1;			///< 右手の中指の付け根の骨の番号（銃を握る位置に使う）
};
