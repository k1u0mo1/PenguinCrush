/**
 * @file   PlayerStats.h
 * @brief  プレイヤーのパラメータを管理するクラス
 * @author 國田知睦
 * @date   2026/06/04
 */

#pragma once

#include <algorithm>
#include <Game/SoundList/AudioManager.h>

struct PlayerStats
{
	//プレイヤーのパラメータ

	//HP関連の初期値
	static constexpr float DEFAULT_HP = 300.0f;
	static constexpr float DEFAULT_HP_MAX = 300.0f;

	//スタミナ関連の初期値
	static constexpr float DEFAULT_STAMINA = 100.0f;
	static constexpr float DEFAULT_STAMINA_MAX = 100.0f;

	//スタミナ回復関連のパラメータ
	//スタミナの回復速度（1秒あたりの回復量）
	static constexpr float STAMINA_RECOVER_RATE = 100.0f; 
	//スタミナを消費したあと、回復が始まるまでのクールタイム（秒）
	static constexpr float STAMINA_COOL_TIME = 1.0f;


	//プレイヤーの体力
	float hp = DEFAULT_HP;
	//プレイヤーの体力（最大）
	float hp_Max = DEFAULT_HP_MAX;

	//プレイヤーのスタミナ
	float stamina = DEFAULT_STAMINA;
	//プレイヤーのスタミナ（最大）
	float stamina_Max = DEFAULT_STAMINA_MAX;

	//回復が始まるまでのタイマー
	float staminaRecoveryCoolTimer = 0.0f;

	//---------------------------------------------------

	/// <summary>
	/// プレイヤーにダメージ
	/// </summary>
	/// <param name="amount">受けるダメージ量</param>
	void TakeDamage(float amount)
	{
		hp = std::max(0.0f, hp - amount);
	}

	/// <summary>
	/// プレイヤーのスタミナを自然回復
	/// </summary>
	/// <param name="dt">前フレームからの経過時間</param>
	void RecoverStamina(float dt)
	{
		//クールタイマー中は回復しない
		if (staminaRecoveryCoolTimer > 0.0f)
		{
			staminaRecoveryCoolTimer -= dt;
			return;
		}

		stamina = std::min(stamina_Max, stamina + dt * STAMINA_RECOVER_RATE);
	}

	/// <summary>
	/// プレイヤーのスタミナを消費
	/// </summary>
	/// <param name="amount">消費するスタミナ量</param>
	void UseStamina(float amount)
	{
		stamina = std::max(0.0f, stamina - amount);

		//スタミナを消費したら回復のクールタイムをリセット
		staminaRecoveryCoolTimer = STAMINA_COOL_TIME;
	}

	/// <summary>
	/// プレイヤーのHPを回復
	/// </summary>
	/// <param name="amount">回復するHP量</param>
	void Heal(float amount)
	{
		//HPを増やす（回復）
		hp = std::min(hp_Max, hp + amount);
	}

	/// <summary>
	/// プレイヤーの体力が0以下になったかを判定
	/// </summary>
	/// <returns>体力が0以下なら true</returns>
	bool IsDead() const
	{
		return hp <= 0.0f;
	}
};
