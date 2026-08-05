
/**
 * @file   CharacterBase.h
 * @brief  キャラクター共通の情報の管理を行うクラス
 * @author 國田知睦
 * @date   2026/07/01
 */

#pragma once
#include "pch.h"
#include <memory>

class ModelCollision;
class Stage;
class Particle;

class CharacterBase
{
protected:

	//重力の強さ
	static constexpr float GRAVITY_FORCE = -100.8f;
	//ノックバックの力
	static constexpr float KNOCKBACK_DRAG = 10.0f;
	//落下しているとみなすY座標
	static constexpr float FALL_LIMIT_Y = -5.0f;
	//落下ダメージ
	static constexpr float FALL_DAMAGE = 20.0f;
	//水しぶきの粒子数
	static constexpr int SPLASH_PARTICLE_COUNT = 20;
	//水面の基本的な高さ
	static constexpr float WATER_SURFACE_Y = 0.0f;

public:
	
	/// <summary>
	/// コンパイラによる既定実装を使用することを明示したデフォルトコンストラクタ
	/// </summary>
	CharacterBase() = default;

	/// <summary>
	///	デストラクタ（バーチャル）
	/// </summary>
	virtual ~CharacterBase() = default;

	/// <summary>
	/// オブジェクトの座標を取得
	/// </summary>
	/// <returns>現在の座標</returns>
	const DirectX::SimpleMath::Vector3& GetPosition() const { return m_position;}

	/// <summary>
	/// オブジェクトの体力が０かどうか
	/// </summary>
	/// <returns>体力が０以下ならtrue　生存していたらfalse</returns>
	bool IsDead() const { return m_hp <= 0.0f; }

	/// <summary>
	/// 共通
	/// </summary>
	/// <param name="dt">経過時間</param>
	/// <param name="stage">ステージオブジェクトのポインタ</param>
	void UpdatePhysice(float dt, Stage* stage);

	/// <summary>
	/// ノックバック
	/// </summary>
	/// <param name="direction">吹っ飛ぶ方向</param>
	/// <param name="power">吹っ飛ぶ強さ</param>
	/// <param name="upPower">上に浮き上がる力</param>
	/// <param name="duration">持続時間</param>
	void ApplyKnockback(
		const DirectX::SimpleMath::Vector3& direction,
		float power,
		float upPower,
		float duration
	);

	/// <summary>
	/// 落下の判定と処理
	/// </summary>
	/// <param name="stage">ステージ</param>
	/// <param name="particle">パーティクル</param>
	void CheckAndHandleFalling(Stage* stage, Particle* particle);

protected:

	//落下しているとみなすY座標を取得する関数
	virtual float GetFallLimitY() const { return FALL_LIMIT_Y; }
	//落下ダメージを取得する関数
	virtual float GetFallDamage() const { return FALL_DAMAGE; }
	//水しぶきの粒子数を取得する関数
	virtual int GetSplashParticleCount() const { return SPLASH_PARTICLE_COUNT; }
	//リスポーンする高さを取得する関数
	virtual DirectX::SimpleMath::Vector3 GetRespawnPosition() const { return DirectX::SimpleMath::Vector3::Zero; }

	//落下ダメージを与える仲介するための関数
	virtual void ExecuteFallDamage()
	{
	}

	//水面の高さを返す
	virtual float GetWaterSurfaceY() const { return WATER_SURFACE_Y; }

protected:

	//座標
	DirectX::SimpleMath::Vector3 m_position = { 0,0,0 };
	//速度
	DirectX::SimpleMath::Vector3 m_velocity = { 0,0,0 };
	//ノックバックの速度
	DirectX::SimpleMath::Vector3 m_knockbackVelocity = { 0,0,0 };

	//ノックバック時間
	float m_knockbackTimer = 0.0f;

	//体力
	float m_hp = 0.0f;
	//体力（最大）
	float m_maxHp = 0.0f;
	//速さ
	float m_speed = 0.0f;

	std::unique_ptr<ModelCollision> m_collision;
};