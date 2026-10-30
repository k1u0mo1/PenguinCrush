
/**
 * @file   GameJudge.h
 * @brief  勝敗ロジック（勝敗判定）クラス
 * @author 國田知睦
 * @date   2026/10/23
 */

#pragma once
#include "Game/PlayerList/Player.h"
#include "Game/EnemyList/EnemyManager.h"


/// <summary>
/// 勝敗判定・シーン切り替えのタイミングを管理
/// </summary>
class GameJudge
{
private:
	//終了時の待機時間
	static constexpr float FINISH_WAIT_TIME = 3.0f;
public:

	/// <summary>
	/// コンストラクタ　デフォルト
	/// </summary>
	GameJudge() = default;

	/// <summary>
	/// 勝利フラグとタイマーのリセット（初期化）
	/// </summary>
	void Reset();
	
	/// <summary>
	/// プレイヤーと敵の状態を見て、勝敗判定を更新
	/// </summary>
	/// <param name="elapsedTime">前フレームからの経過時間</param>
	/// <param name="player">プレイヤーのポインタ</param>
	/// <param name="enemyManager">敵マネージャーのポインタ</param>
	/// <param name="isOpening">オープニング演出かどうか</param>
	void Update(
		float elapsedTime,
		Player* player,
		EnemyManager* enemyManager, 
		bool isOpening
	);

	/// <summary>
	/// 勝敗が決まったか　
	/// プレイヤーがやられたか・敵が全滅したか
	/// </summary>
	/// <returns>ゲームが終了したらtrue</returns>
	bool IsFinised() const { return m_isFinished; }

	/// <summary>
	/// プレイヤーがクリアしたかを取得
	/// </summary>
	/// <returns>クリアしていたらtrue</returns>
	bool IsStageClear() const { return m_isStageClear; }

	/// <summary>
	/// シーン切り替えのタイミング
	/// ゲームシーンー＞リザルトシーン
	/// </summary>
	/// <returns>勝敗の決定後待機時間が経過していれば</returns>
	bool ShouldChangeScene() const { return m_isFinished && m_finishTimer <= 0.0f; }

private:
	//勝敗が決まったか
	bool m_isFinished = false;
	//ステージをクリアしたか
	bool m_isStageClear = false;
	//勝敗の決定後のカウントダウンタイマー
	float m_finishTimer = FINISH_WAIT_TIME;
};