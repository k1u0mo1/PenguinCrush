
/**
 * @file   EnemyBaseParameter.h
 * @brief  敵の基本的なパラメーターをまとめるクラス
 * @author 國田知睦
 * @date   2026/06/30
 */


#pragma once

/// <summary>
/// 敵の基本的なパラメーターをまとめる構造体
/// </summary>
struct EnemyBaseParameter
{
	//体力
	float maxHp;
	//移動速度
	float speed;
	//攻撃のクールタイム
	float attackCooldown;
	//突進攻撃のクールタイム
	float rushCooldown;
	//近距離攻撃のダメージ
	float attackDamage;
	//突進攻撃のダメージ
	float rushDamage;
	//モデルのサイズ
	float scale;

	//--敵が攻撃を食らった際のノックバックのパラメータ--//
	//敵が後ろの方向の吹っ飛ぶ倍率
	float receivedKnockbackPowerMultiplier;
	//敵が上方向への浮き上がり力
	float receivedKnockbackUpwardForce;
	//敵がノックバックの硬直時間
	float receivedKnockbackDuration;
};

//敵の基本的なパラメーターをまとめる名前空間
namespace EnemyData
{

	/// <summary>
	/// ボスの基本的なパラメーター　
	/// </summary>
	static constexpr EnemyBaseParameter BossEnemy =
	{
		//体力
		3000.0f,
		//移動速度
		5.0f,
		//攻撃のクールタイム
		1.5f,
		//突進攻撃のクールタイム
		7.0f,
		//近距離攻撃のダメージ
		20.0f,
		//突進攻撃のダメージ
		30.0f,
		//モデルのサイズ
		2.0f,
		//敵が後ろの方向の吹っ飛ぶ倍率
		10.0f,
		//敵が上方向への浮き上がり力
		25.0f,
		//敵がノックバックの硬直時間
		0.3f
	};

	/// <summary>
	/// 通常敵の基本的なパラメーター　
	/// </summary>
	static constexpr EnemyBaseParameter NormalEnemy =
	{
		//体力
		300.0f,
		//移動速度
		10.0f,
		//攻撃のクールタイム
		2.0f,
		//突進攻撃のクールタイム
		10.0f,
		//近距離攻撃のダメージ
		20.0f,
		//突進攻撃のダメージ
		15.0f,
		//モデルのサイズ
		1.0f,
		//敵が後ろの方向の吹っ飛ぶ倍率
		5.0f,
		//敵が上方向への浮き上がり力
		30.0f,
		//敵がノックバックの硬直時間
		1.0f
	};

	/// <summary>
	/// 通常(大きい)敵の基本的なパラメーター　
	/// </summary>
	static constexpr EnemyBaseParameter BigNormalEnemy =
	{
		//体力
		1000.0f,
		//移動速度
		8.0f,
		//攻撃のクールタイム
		2.0f,
		//突進攻撃のクールタイム
		10.0f,
		//近距離攻撃のダメージ
		50.0f,
		//突進攻撃のダメージ
		30.0f,
		//モデルのサイズ
		1.7f,
		//敵が後ろの方向の吹っ飛ぶ倍率
		8.0f,
		//敵が上方向への浮き上がり力
		30.0f,
		//敵がノックバックの硬直時間
		0.8f
	};

}

