#pragma once
#include "param/GameParam.h"

/**
 * 敵が狙っている間に出す予告線。砲口から狙いの向きへ、決められた長さ（壁があれば壁まで）の赤い線を伸ばす。EnemyGun が持つ
 * 細長いモデルを伸ばして描く考え方は GrappleWire と同じ。持ち主が隠れたら一緒に消えるように、GameObject にせず持ち主の Render で描く
 */
class WarningLine
{
public:
	/** モデルとシェーダーを読む */
	void Init();
	/**
	 * 毎フレーム呼ぶ。isVisible が false の間は出さない
	 * start は線の始まり（砲口）、direction は向き（長さは何でもよい。中で長さ 1 にする）、length は壁がないときの長さ
	 */
	void Update(const bool isVisible, const Vector3& start, const Vector3& direction, const float length);
	/** 出しているときだけ描く */
	void Draw(RenderContext& rc);

private:
	/** シェーダーに渡す値（warningLine.fx の WarningLineCb と同じ並び。16 バイトずつにそろえる） */
	struct Constant
	{
		Vector3 cameraPos;				///< カメラの位置（近い所ほど薄くするのに使う）
		float brightness = 0.0f;		///< 光の強さ
		float time = 0.0f;				///< 出始めからの秒数（ちらつきに使う）
		float padding[3] = {};			///< 16 バイトにそろえるための空き
	};

	static constexpr float MODEL_LENGTH = 100.0f;		///< モデルの元の長さ（Z 方向。原点は片方の端）
	static constexpr float MODEL_DIAMETER = 2.0f;		///< モデルの元の太さ（直径）
	static constexpr float MIN_LENGTH = 0.001f;			///< これより短いと向きを出せないので描かない

	const WarningLineParam& m_param = nsGameParam::Get().warningLine;	///< 調整用の値（Assets/param/warning_line.json）

	ModelRender m_modelRender;			///< 見た目
	Constant m_constant;				///< m_modelRender が描くたびにシェーダーへ送る（Init の後は動かさない）
	float m_visibleTime = 0.0f;			///< 出し始めてからの秒数（出ていない間は 0）
	bool m_canDraw = false;				///< 今のフレームで描くか
};
