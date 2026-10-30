
/**
 * @file   EnemyAttackPattern.cpp
 * @brief  敵の各攻撃パターンの管理を行うクラス
 * @author 國田知睦
 * @date   2026/10/21
 */

#include "pch.h"
#include "EnemyAttackPattern.h"
#include "Game/EnemyList/EnemyManager.h"
#include "Game/EnemyList/EnemyBase.h"

 //----------------------------------------------------------
 // 敵の近距離攻撃のパターンを実行する関数
 //----------------------------------------------------------

void MeleeAttackPattern::Execute(
	EnemyBase* owner,
	const DirectX::SimpleMath::Vector3& forward,
	EnemyManager* enemyManager)
{
	//近距離攻撃のリクエストをEnemyManagerに送る
	if(enemyManager&& owner)
	{
		//攻撃判定オブジェクトの生成リクエストを出す
		enemyManager->RequestAttackE(owner, forward);

		//技を出したのでタイマーをセット
		m_cooldownTimer = COOLDOWN;
	}
}

//----------------------------------------------------------
// 敵の突進攻撃のパターンを管理するクラス
//----------------------------------------------------------

void RushAttackPattern::Execute(
	EnemyBase* owner, 
	const DirectX::SimpleMath::Vector3& forward, 
	EnemyManager* enemyManager)
{
	//突進攻撃のリクエストをEnemyManagerに送る
	if(enemyManager&& owner)
	{
		//攻撃判定オブジェクトの生成リクエストを出す
		enemyManager->RequestRushE(owner, forward);

		//技を出したのでタイマーをセット
		m_cooldownTimer = COOLDOWN;
	}
}

//----------------------------------------------------------
// 敵の範囲攻撃のパターンを管理するクラス
//----------------------------------------------------------

void ColdAuraAttackPattern::Execute(
	EnemyBase* owner,
	const DirectX::SimpleMath::Vector3& forward,
	EnemyManager* enemyManager)
{
	if (enemyManager && owner)
	{
		//攻撃判定オブジェクトの生成リクエストを出す
		enemyManager->RequestColdAuraE(owner, forward);

		//技を出したのでタイマーをセット
		m_cooldownTimer = COOLDOWN;
	}
}
