#pragma once
#include "hud/HudShape.h"
#include "hud/HudText.h"

class Fade;

/**
 * リザルト画面（企画書の「成功」の絵）。タイム・平均速度 Lv・倒した数を 1 行ずつ出し、最後に金の枠の RANK を押す
 * ランクはタイムの点と平均速度 Lv の点を混ぜて決める（境目は result.json）。A（Start）でタイトルへ戻る
 * Game がゴールの後に作り、SetResult で結果を渡す
 */
class ResultScene : public IGameObject
{
public:
	bool Start() override;
	void Update() override;
	void Render(RenderContext& rc) override;

public:
	/** 結果を受け取る（NewGO の直後、Start より前に呼ぶ） */
	void SetResult(const float clearTime, const float averageLevel, const int defeatCount);

private:
	/** タイムと平均速度 Lv から、ランクの文字（S・A・B・C）を決める */
	char CalcRank() const;
	/** 見出しと値を 1 行出す。appearTime 秒から出し始める */
	void DrawRow(HudText& label, HudText& value, const float y, const float appearTime);

private:
	static constexpr float FADE_TIME = 0.4f;			///< 黒くする・戻す秒数
	static constexpr float ROW_INTERVAL = 0.35f;		///< 行を 1 つずつ出す間隔（秒）
	static constexpr float FIRST_ROW_TIME = 0.5f;		///< 最初の行を出す秒数
	static constexpr float RANK_TIME = 1.8f;			///< ランクを押す秒数
	static constexpr float STAMP_TIME = 0.18f;			///< ランクが大きいところから縮んで押されるまでの秒数
	static constexpr float STAMP_SCALE = 2.2f;			///< 押す前の大きさ
	static constexpr float FLASH_TIME = 0.4f;			///< 押した後に枠が白く光る秒数
	static constexpr float INPUT_TIME = 2.4f;			///< この秒数からボタンを受け付ける（ランクを見る前に抜けないように）
	static constexpr float HEADER_HEIGHT = 96.0f;		///< 「RESULT」の文字の高さ
	static constexpr float SUB_HEIGHT = 28.0f;			///< 「FIREWALL BREACHED」の文字の高さ
	static constexpr float LEFT_X = -800.0f;			///< 左の列の左端
	static constexpr float VALUE_RIGHT_X = 160.0f;		///< 値の右端
	static constexpr float HEADER_Y = 380.0f;			///< 「RESULT」の位置
	static constexpr float SUB_Y = 300.0f;				///< 「FIREWALL BREACHED」の位置
	static constexpr float LABEL_HEIGHT = 32.0f;		///< 見出しの文字の高さ
	static constexpr float VALUE_HEIGHT = 72.0f;		///< 値の文字の高さ
	static constexpr float ROW_TOP_Y = 150.0f;			///< いちばん上の行の位置
	static constexpr float ROW_SPACING = 130.0f;		///< 行の間隔
	static constexpr float RANK_X = 560.0f;				///< ランクの枠の中心
	static constexpr float RANK_Y = 20.0f;
	static constexpr float RANK_BOX_WIDTH = 440.0f;		///< ランクの枠の大きさ
	static constexpr float RANK_BOX_HEIGHT = 520.0f;
	static constexpr float RANK_LABEL_HEIGHT = 40.0f;	///< 「RANK」の文字の高さ
	static constexpr float RANK_LABEL_Y = 190.0f;		///< 「RANK」の、枠の中心からの高さ
	static constexpr float RANK_LETTER_HEIGHT = 320.0f;	///< ランクの文字の高さ
	static constexpr float RANK_LETTER_Y = -30.0f;		///< ランクの文字の、枠の中心からの高さ
	static constexpr float PRESS_HEIGHT = 34.0f;		///< 「PRESS [A] TITLE」の文字の高さ
	static constexpr float PRESS_Y = -430.0f;			///< 「PRESS [A] TITLE」の位置
	static constexpr float BLINK_SPEED = 3.0f;			///< 点滅の速さ（ラジアン / 秒）
	static constexpr float GRID_SPEED = 0.6f;			///< 背景の格子の流れる速さ（タイトルよりゆっくり）
	static constexpr float SECONDS_PER_MINUTE = 60.0f;	///< 1 分の秒数
	static constexpr float MIN_LEVEL = 1.0f;			///< 速度 Lv のいちばん下
	static constexpr float MAX_LEVEL = 5.0f;			///< 速度 Lv のいちばん上

	Fade* m_fade = nullptr;				///< 画面を黒くする係（main が作ったものを借りる）
	bool m_isLeaving = false;			///< ボタンが押され、黒くしている途中か
	bool m_isStamped = false;			///< ランクを押したか（押した瞬間に音を鳴らすため）
	float m_time = 0.0f;				///< 出てからの秒数
	float m_clearTime = 0.0f;			///< クリアしたタイム（秒）
	float m_averageLevel = 1.0f;		///< 平均の速度 Lv
	int m_defeatCount = 0;				///< 倒した敵の数
	char m_rank = 'C';					///< ランクの文字
	HudShape m_back;					///< 背景（Assets/shader/sceneBack.fx）
	HudShape m_rankBox;					///< ランクの金の枠（Assets/shader/resultRank.fx）
	HudText m_header;					///< 「RESULT」
	HudText m_sub;						///< 「FIREWALL BREACHED」
	HudText m_timeLabel;				///< 「TIME」
	HudText m_timeValue;				///< タイムの数字
	HudText m_levelLabel;				///< 「AVG SPEED LV」
	HudText m_levelValue;				///< 平均の速度 Lv
	HudText m_defeatLabel;				///< 「DEFEATS」
	HudText m_defeatValue;				///< 倒した数
	HudText m_rankLabel;				///< 「RANK」
	HudText m_rankLetter;				///< ランクの文字
	HudText m_press;					///< 「PRESS [A] TITLE」
};
