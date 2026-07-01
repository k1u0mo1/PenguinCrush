
/**
 * @file   CharacterBase.cpp
 * @brief  キャラクター共通の情報の管理を行うクラス
 * @author 國田知睦
 * @date   2026/07/01
 */

#include "pch.h"
#include "CharacterBase.h"
#include "Game/GimmickList/Stage.h"
#include "Game/SoundList/AudioManager.h" 
#include "Game/Effects/Particle.h"

//----------------------------------------------------------
// キャラクター共通
//----------------------------------------------------------

void CharacterBase::UpdatePhysice(float dt, Stage* stage)
{
	//重力の適用 
	m_velocity.y += GRAVITY_FORCE * dt;
	m_position.y += m_velocity.y * dt;

	//ノックバックの適用
	if (m_knockbackTimer > 0.0f)
	{
		//ノックバック
		m_knockbackTimer -= dt;
		m_knockbackVelocity -= m_knockbackVelocity * KNOCKBACK_DRAG * dt;

		m_position.x += m_knockbackVelocity.x * dt;
		m_position.z += m_knockbackVelocity.z * dt;

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
}

//----------------------------------------------------------
// ノックバック共通
//----------------------------------------------------------

void CharacterBase::ApplyKnockback(
	const DirectX::SimpleMath::Vector3& direction,
	float power,
	float upPower,
	float duration)
{
	m_knockbackVelocity = direction * power;
	m_knockbackVelocity.y = upPower;

	m_knockbackTimer = duration;
}

//----------------------------------------------------------
// 落下の判定と処理共通
//----------------------------------------------------------

void CharacterBase::CheckAndHandleFalling(
	Stage* /*stage*/, 
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

		//リスポーン位置に移動させる
		m_position = GetRespawnPosition();
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