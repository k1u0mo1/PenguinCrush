
/**
 * @file   EnemyAttackPattern.h
 * @brief  敵の各攻撃パターンの管理を行うクラス
 * @author 國田知睦
 * @date   2026/06/04
 */

#pragma once
#include "pch.h"

class EnemyBase;
class EnemyManager;

/// <summary>
/// 敵の攻撃パターンを管理する基底クラス
/// </summary>
class EnemyAttackPattern
{
public:

	/// <summary>
	/// 敵の攻撃パターンのデストラクタ
	/// </summary>
	virtual ~EnemyAttackPattern() = default;

	/// <summary>
	/// 敵の攻撃を実行するための純粋仮想関数
	/// </summary>
	/// <param name="owner">攻撃を行う敵のインスタンス</param>
	/// <param name="forward">攻撃の方向ベクトル</param>
	/// <param name="enemyManager">敵の管理クラス</param>
	virtual void Execute(
		EnemyBase* owner,
		const DirectX::SimpleMath::Vector3& forward,
		EnemyManager* enemyManager
		) = 0;

};

#include "Game/EnemyList/EnemyManager.h"

/// <summary>
/// 敵の近距離攻撃のパターンを管理するクラス
/// </summary>
class MeleeAttackPattern : public EnemyAttackPattern
{
public:

	/// <summary>
	/// 敵の近距離攻撃のパターンを実行する関数
	/// </summary>
	/// <param name="owner">攻撃を行う敵のインスタンス</param>
	/// <param name="forward">攻撃の方向ベクトル</param>
	/// <param name="enemyManager">敵の管理クラス</param>
	void Execute(
		EnemyBase* owner,
		const DirectX::SimpleMath::Vector3& forward,
		EnemyManager* enemyManager
	)override;
};

/// <summary>
/// 敵の突進攻撃のパターンを管理するクラス
/// </summary>
class RushAttackPattern : public EnemyAttackPattern
{
public:

	void Execute(
		EnemyBase* owner,
		const DirectX::SimpleMath::Vector3& forward,
		EnemyManager* enemyManager
	)override;
};