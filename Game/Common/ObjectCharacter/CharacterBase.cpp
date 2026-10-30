
/**
 * @file   CharacterBase.cpp
 * @brief  キャラクター共通の情報の管理を行うクラス
 * @author 國田知睦
 * @date   2026/10/23
 */

#include "pch.h"
#include "CharacterBase.h"
#include "Game/GimmickList/Stage.h"
#include "Game/SoundList/AudioManager.h" 
#include "Game/Effects/Particle.h"

//----------------------------------------------------------
// キャラクター共通
//----------------------------------------------------------

void CharacterBase::UpdatePhysice(
	float dt,
	Stage* stage,
	Particle* particle
)
{
	//重力の適用 
	m_velocity.y += GRAVITY_FORCE * dt;
	m_position   += m_velocity * dt;

	//ノックバックの適用
	if (m_knockbackTimer > 0.0f)
	{
		//ノックバック
		m_knockbackTimer -= dt;

		//XZ方向の移動速度を減速
		m_knockbackVelocity.x -= m_knockbackVelocity.x * KNOCKBACK_DRAG * dt;
		m_knockbackVelocity.z -= m_knockbackVelocity.z * KNOCKBACK_DRAG * dt;

		//Y方向に重力を与え、放物線にする
		m_knockbackVelocity.y += GRAVITY_FORCE * dt;

		//座標へ移動量を加算
		m_position += m_knockbackVelocity * dt;

		if(m_knockbackTimer <= 0.0f)
		{
			m_knockbackVelocity = DirectX::SimpleMath::Vector3::Zero;
		}
	}

	//接地判定（めり込まないように）
	if (stage)
	{
		float stageY = stage->GetGroundHeight(m_position.x, m_position.z);

		//足場より下なら着地
		if (m_position.y < stageY)
		{
			m_position.y = stageY;
			m_knockbackVelocity.y = 0.0f;

			//【保険】着地しら落下速度をリセット
			m_velocity.y = 0.0f;
		}
	}

	CheckAndHandleFalling(stage, particle);
}

//----------------------------------------------------------
// ノックバック共通
//----------------------------------------------------------

void CharacterBase::ApplyKnockback(
	const DirectX::SimpleMath::Vector3& direction,
	float power, 
	float powerMultiplier,
	float upwardForce,
	float duration
)
{
	//水平方向のベクトルを正規化　→
	DirectX::SimpleMath::Vector3 horizDir = direction;
	horizDir.y = 0.0f;
	if (horizDir.LengthSquared() > 0.0001f)
	{
		horizDir.Normalize();
	}

	//水平方向の吹き飛び初速
	m_knockbackVelocity = horizDir * (power * powerMultiplier);

	//垂直方向の初速を設定　↑
	m_knockbackVelocity.y = upwardForce;
	
	//硬直タイマーの設定
	m_knockbackTimer = duration;
}

//----------------------------------------------------------
// 落下の判定と処理共通
//----------------------------------------------------------

void CharacterBase::CheckAndHandleFalling(
	Stage* stage, 
	Particle* particle)
{
	//落下しているとみなすY座標を下回ったら落ちたとみなす
	if (m_position.y < GetFallLimitY())
	{
		//水しぶきのエフェクトを発生させる
		if (particle)
		{
			//水しぶきの位置はキャラクターのX,Z座標で、Y座標は地面と同じにする
			DirectX::SimpleMath::Vector3 splashPosition = m_position;
			//水面の高さをもらう
			splashPosition.y = GetWaterSurfaceY();
			//水しぶきを発生させる
			particle->Spawn(Particle::Type::Splash, splashPosition, GetSplashParticleCount());
		}

		//安全な位置にリスポーンするため
		DirectX::SimpleMath::Vector3 respawnPos;

		//ステージがあり、安全な足場が見つかった
		if (stage &&
			stage->FindSafeRespawnPosition(
				GetRespawnPosition().y,
				GetFallLimitY(), 
				respawnPos)
			)
		{
			m_position = respawnPos;
		}
		else
		{
			//見つからないとデフォルトの座標
			m_position = GetRespawnPosition();
		}

		//複数体が重なるのを防ぐ
		float offsetX = ((static_cast<float>(rand()) / RAND_MAX) - RESPAWN_RANGE) * RESPAWN_OFFSET_RANGE;
		float offsetZ = ((static_cast<float>(rand()) / RAND_MAX) - RESPAWN_RANGE) * RESPAWN_OFFSET_RANGE;
		m_position.x += offsetX;
		m_position.z += offsetZ;


		//リセット
		m_velocity = DirectX::SimpleMath::Vector3::Zero;
		m_knockbackVelocity = DirectX::SimpleMath::Vector3::Zero;
		m_knockbackTimer = 0.0f;

		//ダメージを与える
		ExecuteFallDamage();
		//落ちたときのSEを再生
		AudioManager::GetInstance()->Play("Fall");
	}
}