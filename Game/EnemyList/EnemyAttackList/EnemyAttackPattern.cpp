
/**
 * @file   EnemyAttackPattern.cpp
 * @brief  敵の各攻撃パターンの管理を行うクラス
 * @author 國田知睦
 * @date   2026/06/04
 */

#include "pch.h"
#include "EnemyAttackPattern.h"
#include "Game/EnemyList/EnemyManager.h"
#include "Game/EnemyList/EnemyBase.h"


void MeleeAttackPattern::Execute(
	EnemyBase* owner,
	const DirectX::SimpleMath::Vector3& forward,
	EnemyManager* enemyManager)
{
	//近距離攻撃のリクエストをEnemyManagerに送る
	if(enemyManager&& owner)
	{
		enemyManager->RequestAttackE(owner, forward);
	}
}

void RushAttackPattern::Execute(
	EnemyBase* owner, 
	const DirectX::SimpleMath::Vector3& forward, 
	EnemyManager* enemyManager)
{
	//突進攻撃のリクエストをEnemyManagerに送る
	if(enemyManager&& owner)
	{
		enemyManager->RequestRushE(owner, forward);
	}
}
