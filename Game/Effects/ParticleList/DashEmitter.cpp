
/**
 * @file   DashEmitter.cpp
 * @brief  落下した際の水しぶき専用　パーティクルクラス
 * @author 國田知睦
 * @date   2026/07/15
 */

#include "pch.h"
#include "DashEmitter.h"
#include <algorithm>

//----------------------------------------------------------
//コンストラク　初期化・発生処理
//----------------------------------------------------------

DashEmitter::DashEmitter(
	DirectX::SimpleMath::Vector3 position, int count)
{
	for (int i = 0; i < count; i++)
	{
		ParticleInfo p;
		p.Position = position;
		p.Age = 0.0f;
		p.Size = EFFECT_SIZE;
		p.Color = BASE_COLOR;
		//寿命をばらつかせる
		p.Lifetime = GetRandom(LIFETIME_MIN, LIFETIME_MAX);

		//位置を少しばらつかせる
		p.Position.x += GetRandom(POSITION_XZ_MIN, POSITION_XZ_MAX);
		p.Position.y += GetRandom(POSITION_Y_MIN, POSITION_Y_MAX);
		p.Position.z += GetRandom(POSITION_XZ_MIN, POSITION_XZ_MAX);

		//速度を設定
		p.Velocity.x = GetRandom(VELOCITY_XZ_MIN, VELOCITY_XZ_MAX);
		p.Velocity.y = GetRandom(VELOCITY_Y_MIN, VELOCITY_Y_MAX);
		p.Velocity.z = GetRandom(VELOCITY_XZ_MIN, VELOCITY_XZ_MAX);

		m_particles.push_back(p);
	}
}

//----------------------------------------------------------
//更新処理
//----------------------------------------------------------

void DashEmitter::Update(float elapsedTime)
{
	for (auto& p : m_particles)
	{
		
		p.Velocity.y += VELOCITY_DECAY * elapsedTime;
		p.Position += p.Velocity * elapsedTime;
		p.Age += elapsedTime;
	}

	//寿命切れのパーティクルを削除
	m_particles.erase(
		std::remove_if(m_particles.begin(), m_particles.end(),
			[](const ParticleInfo& p) {return p.Age >= p.Lifetime; }),
		m_particles.end());
	
}
