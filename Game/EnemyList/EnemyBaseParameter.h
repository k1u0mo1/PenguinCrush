
/**
 * @file   EnemyBaseParameter.h
 * @brief  敵の基本的なステータス・物理パラメータをまとめるクラス
 * @author 國田知睦
 * @date   2026/10/22
 */

#pragma once

/// <summary>
/// 敵の基本的なパラメーターをまとめる構造体
/// </summary>
struct EnemyBaseParameter
{
	float maxHp;	//体力
	float speed;	//移動速度
	float scale;	//モデルのサイズ

	//--敵が攻撃を食らった際のノックバックのパラメータ--//
	float receivedKnockbackPowerMultiplier;	//敵が後ろの方向の吹っ飛ぶ倍率
	float receivedKnockbackUpwardForce;		//敵が上方向への浮き上がり力
	float receivedKnockbackDuration;		//敵がノックバックの硬直時間
};

//敵の基本的なパラメーターをまとめる名前空間
namespace EnemyData
{
	/// <summary>
	/// ボスの基本的なパラメーター　
	/// </summary>
	static constexpr EnemyBaseParameter BossEnemy =
	{
		3000.0f,//体力
		5.0f,	//移動速度
		2.0f,	//モデルのサイズ
		10.0f,	//敵が後ろの方向の吹っ飛ぶ倍率
		25.0f,	//敵が上方向への浮き上がり力
		0.3f	//敵がノックバックの硬直時間
	};

	/// <summary>
	/// 通常敵の基本的なパラメーター　
	/// </summary>
	static constexpr EnemyBaseParameter NormalEnemy =
	{
		300.0f,	//体力
		10.0f,	//移動速度
		1.0f,	//モデルのサイズ
		5.0f,	//敵が後ろの方向の吹っ飛ぶ倍率
		30.0f,	//敵が上方向への浮き上がり力
		1.5f	//敵がノックバックの硬直時間
	};

	/// <summary>
	/// 通常(大きい)敵の基本的なパラメーター　
	/// </summary>
	static constexpr EnemyBaseParameter BigNormalEnemy =
	{
		1000.0f,//体力
		8.0f,	//移動速度
		1.7f,	//モデルのサイズ
		8.0f,	//敵が後ろの方向の吹っ飛ぶ倍率
		30.0f,	//敵が上方向への浮き上がり力
		0.8f	//敵がノックバックの硬直時間
	};
}

