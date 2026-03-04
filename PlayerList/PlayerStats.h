
//PlayerStats.h

//プレイヤーのパラメータ管理

#pragma once
#include <algorithm>
#include <SoundList/AudioManager.h>

struct PlayerStats
{
	//プレイヤーの体力
	float hp = 300.0f;
	//プレイヤーの体力（最大）
	float hp_Max = 300.0f;

	//プレイヤーのスタミナ
	float stamina = 100.0f;
	//プレイヤーのスタミナ（最大）
	float stamina_Max = 100.0f;

	//プレイヤーの弾
	int ammo = 10;
	//プレイヤーの弾（最大）
	int ammo_Max = 10;


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

		stamina = std::min(stamina_Max, stamina + dt * 100.0f);
	}

	/// <summary>
	/// プレイヤーのスタミナを消費
	/// </summary>
	/// <param name="amount">消費するスタミナ量</param>
	void UseStamina(float amount)
	{
		stamina = std::max(0.0f, stamina - amount);

		//スタミナを使ったあと、〇.〇秒間は回復しないようにする
		staminaRecoveryCoolTimer = 1.0f;
	}

	/// <summary>
	/// 弾を1発ずつ消費
	/// </summary>
	void UseAmmo()
	{
		if (ammo > 0)
		{
			ammo--;
		}
	}

	/// <summary>
	/// 弾を最大までリロード　リセット
	/// </summary>
	void ReloadAmmo()
	{
		ammo = ammo_Max;
		
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
