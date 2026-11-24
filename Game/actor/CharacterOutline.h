#pragma once
#include "param/GameParam.h"

/**
 * キャラの輪郭線。体と同じモデルを法線の向きへ少しふくらませ、裏の面だけを光る色で描く（裏返した殻）
 * 体の外に、ふくらませた分だけ殻のふちがはみ出して線に見える。遠くからでも敵を見つけやすくするため
 * BattleCharacter が持つ（設定に outline があるときだけ）
 */
class CharacterOutline
{
public:
	/** 体と同じモデルを読み込む */
	void Init(const char* modelPath, const CharacterParam::Outline& param);
	/** 体と同じ位置・向き・大きさに置く */
	void Update(const Vector3& position, const Quaternion& rotation, const float scale);
	/** 描く */
	void Draw(RenderContext& rc);

private:
	/** outline.fx に渡す値（outline.fx の OutlineCb と同じ並び） */
	struct Constant
	{
		Vector3 cameraPos;		///< カメラの位置（距離で線の太さを変える）
		float width = 0.0f;		///< 線の太さ（カメラからの距離に掛ける）
		Vector3 color;			///< 線の色
		float pad = 0.0f;		///< 16 バイトにそろえるための詰め物
	};

	ModelRender m_modelRender;		///< 殻のモデル
	Constant m_constant;			///< シェーダーに渡す値（ModelRender が毎フレーム読む）
};
