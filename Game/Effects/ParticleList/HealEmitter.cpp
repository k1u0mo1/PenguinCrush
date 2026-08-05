/**
 * @file   HealEmitter.cpp
 * @brief  回復した時専用　パーティクルクラス
 * @author 國田知睦
 * @date   2026/07/29
 */

#include "pch.h"
#include "HealEmitter.h"
#include <algorithm>

 //----------------------------------------------------------
 //コンストラク　初期化・発生処理
 //----------------------------------------------------------

HealEmitter::HealEmitter(
	DirectX::SimpleMath::Vector3 position, int count)
{
	//一番目のテクスチャの番号を使用するように指定
	m_textureIndex = 1;

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
		p.Position.y += GetRandom(POSITION_Y_MIN, POSITION_Y_MAX);
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

void HealEmitter::Update(float elapsedTime)
{
	for (auto& p : m_particles)
	{
		p.Age += elapsedTime;
		float ratio = p.Age / p.Lifetime;
		
		//上に向かって上昇する
		p.Position.y += p.Velocity.y * elapsedTime;

		//プレイヤーを中心に回転する
		float currentAngle = p.Velocity.x * p.Age;
		float radius = p.Velocity.z;

		//発生した座標からのオフセット 回転
		p.Position.x += -sin(currentAngle) * radius * p.Velocity.x * elapsedTime;
		p.Position.z +=  cos(currentAngle) * radius * p.Velocity.x * elapsedTime;

		p.Size = (EFFECT_BASE_SIZE - ratio) * EFFECT_SIZE;

		
	}

	//寿命切れのパーティクルを削除
	m_particles.erase(
		std::remove_if(m_particles.begin(), m_particles.end(),
			[](const ParticleInfo& p) {return p.Age >= p.Lifetime; }),
		m_particles.end());
}
