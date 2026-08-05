
/**
 * @file   SplashEmitter.cpp
 * @brief  落下した際の水しぶき専用　パーティクルクラス
 * @author 國田知睦
 * @date   2026/07/15
 */

#include "pch.h"
#include "SplashEmitter.h"
#include <algorithm>

//----------------------------------------------------------
//コンストラク　初期化・発生処理
//----------------------------------------------------------

SplashEmitter::SplashEmitter(
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

		//発生座標を少しばらつかせる
		p.Position.x += GetRandom(POSITION_MIN, POSITION_MAX);
		p.Position.z += GetRandom(POSITION_MIN, POSITION_MAX);
		
		//円形に広がる＆上に向かって跳ねる速度
		float speed = GetRandom(CIRCULAR_SPEED_MIN, CIRCULAR_SPEED_MAX);
		float angle = GetRandom(0.0f, DirectX::XM_2PI);
		float radius = GetRandom(CIRCULAR_RADIUS_MIN, CIRCULAR_RADIUS_MAX);

		p.Velocity.x = cos(angle) * radius;
		p.Velocity.y = speed;
		p.Velocity.z = sin(angle) * radius;

		m_particles.push_back(p);
	}
}

//----------------------------------------------------------
//更新処理
//----------------------------------------------------------

void SplashEmitter::Update(float elapsedTime)
{
	for (auto& p : m_particles)
	{
		//水しぶき特有の動き
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
