
/**
 * @file   EnemyAttackPattern.h
 * @brief  敵の各攻撃パターンの管理を行うクラス
 *		   ビヘイビアツリーの役割
 * @author 國田知睦
 * @date   2026/10/21
 */

#pragma once
#include "pch.h"
#include "Game/EnemyList/EnemyBase.h"

class EnemyBase;
class EnemyManager;

/// <summary>
/// 敵の攻撃パターンを管理する基底クラス
/// ビヘイビアツリーAction/Conditionノード
/// </summary>
class EnemyAttackPattern
{
public:

	/// <summary>
	/// 敵の攻撃パターンのデストラクタ
	/// </summary>
	virtual ~EnemyAttackPattern() = default;

	/// <summary>
	/// 攻撃独自のクールダウンタイマーを減算
	/// </summary>
	/// <param name="dt">前フレームからの経過時間</param>
	virtual void UpdateTimer(float dt)
	{
		UNREFERENCED_PARAMETER(dt);
	}

	/// <summary>
	/// 攻撃が実行可能かどうか評価する
	/// Conditionノード
	/// </summary>
	/// <param name="owner">攻撃を行う敵</param>
	/// <param name="dist">プレイヤーとの現在の距離</param>
	/// <returns>距離条件とクールダウンが終われば　true</returns>
	virtual bool CanExecute(EnemyBase* owner, float dist)const = 0;

	/// <summary>
	/// 攻撃の優先度を取得
	/// 数値が大きい程優先される
	/// Selectorノード
	/// </summary>
	/// <returns></returns>
	virtual int GetPriority()const { return 0; }

	/// <summary>
	/// 敵の攻撃を実行するための純粋仮想関数
	/// Actionノード
	/// </summary>
	/// <param name="owner">攻撃を行う敵のインスタンス</param>
	/// <param name="forward">攻撃の方向ベクトル</param>
	/// <param name="enemyManager">敵の管理クラス</param>
	virtual void Execute(
		EnemyBase* owner,
		const DirectX::SimpleMath::Vector3& forward,
		EnemyManager* enemyManager
		) = 0;

	/// <summary>
	/// 攻撃モーション・技の持続時間の取得
	/// </summary>
	/// <returns>持続時間</returns>
	virtual float GetDuration()const = 0;

	/// <summary>
	/// 攻撃時にボスが遷移するアニメーション・ステートを取得
	/// </summary>
	/// <returns>遷移先のEnemyState</returns>
	virtual EnemyState GetState()const = 0;
};

#include "Game/EnemyList/EnemyManager.h"

/// <summary>
/// 敵の近距離攻撃のパターンを管理するクラス
/// </summary>
class MeleeAttackPattern : public EnemyAttackPattern
{
private:
	//クールダウンタイマー
	float m_cooldownTimer = 0.0f;
	//クールダウン時間
	static constexpr float COOLDOWN = 2.5f;
	//技が届く最大距離
	static constexpr float ATTACK_DISTANCE = 10.0f;
	//硬直・攻撃ステートの持続時間
	static constexpr float DURATION = 1.0f;
	//攻撃の優先度
	static constexpr int PRIORITY = 10;
public:
	/// <summary>
	/// クールダウンタイマーの更新
	/// </summary>
	/// <param name="dt">時間</param>
	void UpdateTimer(float dt)override
	{
		if (m_cooldownTimer > 0.0f)
		{
			m_cooldownTimer -= dt;
		}
	}

	/// <summary>
	/// 評価関数
	/// 距離が範囲内・クールダウンが終わっていれば発動可
	/// </summary>
	/// <param name="owner">攻撃を行う敵</param>
	/// <param name="dist">プレイヤーとの現在の距離</param>
	/// <returns>前フレームからの経過時間</returns>
	bool CanExecute(EnemyBase* owner, float dist)const override
	{
		UNREFERENCED_PARAMETER(owner);

		return (dist < ATTACK_DISTANCE && m_cooldownTimer <= 0.0f);
	}

	/// <summary>
	/// 優先度の設定
	/// </summary>
	/// <returns>優先度</returns>
	int GetPriority()const override { return PRIORITY; }

	/// <summary>
	/// 攻撃モーションの持続時間を取得
	/// </summary>
	/// <returns>持続時間</returns>
	float GetDuration()const override { return DURATION; }

	/// <summary>
	/// 攻撃時に遷移するステートを取得
	/// </summary>
	/// <returns>EnemyState::Attack</returns>
	EnemyState GetState()const override { return EnemyState::Attack; }

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
private:
	//クールダウンタイマー
	float m_cooldownTimer = 0.0f;
	//クールダウン時間
	static constexpr float COOLDOWN = 7.0f;
	//技が届く最大距離
	static constexpr float RUSH_DISTANCE = 50.0f;
	//硬直・攻撃ステートの持続時間
	static constexpr float DURATION = 2.0f;
	//攻撃の優先度
	static constexpr int PRIORITY = 10;
public:
	/// <summary>
	/// クールダウンタイマーを設定
	/// </summary>
	/// <param name="dt">前フレームからの経過時間</param>
	void UpdateTimer(float dt)override 
	{
		if (m_cooldownTimer > 0.0f)
		{
			m_cooldownTimer -= dt;
		}
	}

	/// <summary>
	/// 突進攻撃が実行可能か評価
	/// </summary>
	/// <param name="owner">攻撃を行う敵</param>
	/// <param name="dist">レイヤーとの距離</param>
	/// <returns>範囲内・クールダウン終了時</returns>
	bool CanExecute(EnemyBase* owner, float dist)const override
	{
		UNREFERENCED_PARAMETER(owner);

		return (dist < RUSH_DISTANCE && m_cooldownTimer <= 0.0f);
	}

	/// <summary>
	/// 優先度の取得
	/// </summary>
	/// <returns>優先度</returns>
	int GetPriority()const override { return PRIORITY; }

	/// <summary>
	/// 突進モーションの持続時間を取得
	/// </summary>
	/// <returns>持続時間</returns>
	float GetDuration()const override { return DURATION; }

	/// <summary>
	/// 突進時に遷移するステートを取得
	/// </summary>
	/// <returns>EnemyState::Rush</returns>
	EnemyState GetState()const override { return EnemyState::Rush; }

	/// <summary>
	/// 突進攻撃を実行
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
/// 敵の周囲冷気攻撃のパターンを管理するクラス
/// </summary>
class ColdAuraAttackPattern : public EnemyAttackPattern
{
private:
	//クールダウンタイマー
	float m_cooldownTimer = 0.0f;
	//クールダウン時間
	static constexpr float COOLDOWN = 10.0f;
	//技が届く最大距離
	static constexpr float AURA_DISTANCE = 8.0f;
	//硬直・攻撃ステートの持続時間
	static constexpr float DURATION = 1.5f;
	//攻撃の優先度
	static constexpr int PRIORITY = 10;
public:
	/// <summary>
	/// クールダウンタイマーの更新
	/// </summary>
	/// <param name="dt">前フレームからの経過時間</param>
	void UpdateTimer(float dt)override
	{
		if (m_cooldownTimer > 0.0f)
		{
			m_cooldownTimer -= dt;
		}
	}

	/// <summary>
	/// 範囲攻撃が実行可能か評価
	/// </summary>
	/// <param name="owner">攻撃を行う敵</param>
	/// <param name="dist">プレイヤーとの距離</param>
	/// <returns>近距離・クールダウン終了時</returns>
	bool CanExecute(EnemyBase* owner, float dist)const override
	{
		UNREFERENCED_PARAMETER(owner);

		return (dist < AURA_DISTANCE && m_cooldownTimer <= 0.0f);
	}

	/// <summary>
	/// 優先度の取得
	/// </summary>
	/// <returns>優先度</returns>
	int GetPriority()const override { return PRIORITY; }

	/// <summary>
	/// 範囲攻撃モーションの持続時間を取得
	/// </summary>
	/// <returns>持続時間</returns>
	float GetDuration()const override { return DURATION; }

	/// <summary>
	/// 範囲攻撃時に遷移するステートを取得
	/// </summary>
	/// <returns>EnemyState::ColdAura</returns>
	EnemyState GetState()const override { return EnemyState::ColdAura; }

	/// <summary>
	/// 範囲攻撃を実行
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