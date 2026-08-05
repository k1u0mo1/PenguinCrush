
/**
 * @file   SplashEmitter.cpp
 * @brief  攻撃が当たった時の爆発専用　パーティクルクラス
 * @author 國田知睦
 * @date   2026/07/15
 */

#include "pch.h"
#include "ExplosionEmitter.h"
#include <algorithm>

//----------------------------------------------------------
//コンストラク　初期化・発生処理
//----------------------------------------------------------

ExplosionEmitter::ExplosionEmitter(
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

		//球状に弾けるような速度
		float speed = GetRandom(SPEED_MIN, SPEED_MAX);
		//上下の角度
		float theta = GetRandom(0.0f, DirectX::XM_2PI);
		//水平の角度
		float phi = GetRandom(0.0f, DirectX::XM_2PI);

		p.Velocity.x = sin(theta) * cos(phi) * speed;
		p.Velocity.y = cos(theta) * speed;
		p.Velocity.z = sin(theta) * sin(phi) * speed;
		
		m_particles.push_back(p);
	}
}

//----------------------------------------------------------
//更新処理
//----------------------------------------------------------

void ExplosionEmitter::Update(float elapsedTime)
{
	for (auto& p : m_particles)
	{
		p.Velocity *= VELOCITY_DECAY;
		p.Position += p.Velocity * elapsedTime;
		//爆発サイズが徐々に大きくする
		p.Size += SIZE_MAGNIFICATION_RATE * elapsedTime;

		p.Age += elapsedTime;
	}

	//寿命切れのパーティクルを削除
	m_particles.erase(
		std::remove_if(m_particles.begin(), m_particles.end(),
			[](const ParticleInfo& p) {return p.Age >= p.Lifetime; }),
		m_particles.end()
	);
	
}
