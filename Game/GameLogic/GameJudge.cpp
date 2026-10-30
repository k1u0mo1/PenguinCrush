
/**
 * @file   GameJudge.cpp
 * @brief  勝敗ロジック（勝敗判定）クラス
 * @author 國田知睦
 * @date   2026/10/23
 */

#include "pch.h"
#include "GameJudge.h"

#include "Game/EnemyList/BossEnemy.h"

void GameJudge::Reset()
{
	m_isFinished = false;
	m_isStageClear = false;
	m_finishTimer = FINISH_WAIT_TIME;
}

void GameJudge::Update(
	float elapsedTime, 
	Player* player,
	EnemyManager* enemyManager,
	bool isOpening)
{
	if (m_isFinished)
	{
		m_finishTimer -= elapsedTime;
		return;
	}

	//プレイヤーがやられたか判定
	if (player && player->IsDead())
	{
		m_isFinished = true;
		m_isStageClear = false;
		return;
	}

	//敵のクリア判定
	if (enemyManager)
	{
		if (enemyManager->HasBoss())
		{
			//ボス敵の情報を取得
			BossEnemy* boss = enemyManager->GetBoss();
			if (boss && (boss->GetHP()<=0.0f||boss->GetState()==EnemyState::Dying))
			{
				m_isStageClear = true;
			}
		}
		else
		{
			if (!isOpening && enemyManager->GetActiveEnemies().empty())
			{
				m_isStageClear = true;
			}
		}

		if (m_isStageClear)
		{
			m_isFinished = true;
		}
	}
}
