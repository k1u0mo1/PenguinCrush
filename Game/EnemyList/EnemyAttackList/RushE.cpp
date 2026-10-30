/**
 * @file   RushE.cpp
 * @brief  敵の突進攻撃クラス
 * @author 國田知睦
 * @date   2026/10/27
 */

#include "pch.h"
#include "RushE.h"
#include "Game/EnemyList/BossEnemy.h"
#include "Game/EnemyList/EnemyBase.h"
#include <Effects.h>
#include <Game/Common/ObjectCharacter/ModelManager.h>

//----------------------------------------------------------
// 敵の突進攻撃を生成
//----------------------------------------------------------

RushE::RushE(
	DX::DeviceResources* deviceResources,
	EnemyBase* enemy,
	const DirectX::SimpleMath::Vector3& forward,
	std::shared_ptr<DisplayCollision> displayCollision,
	Particle* particle)
	: 
	m_deviceResources(deviceResources),
	m_forward(forward),
	m_lifetime(0.0f),
	m_displayCollision(displayCollision),
	m_enemy(enemy),
	m_particle(particle)

{
	//ボスの位置に当たり判定を作る
	m_position = m_enemy->GetPosition();
	//当たり判定
	m_collision = std::make_unique<ModelCollisionOrientedBox>();
	m_collision->SetCenter(m_position);
	m_collision->SetExtents(COLLISION_SIZE);

	m_states = std::make_unique<DirectX::CommonStates>(m_deviceResources->GetD3DDevice());
}

//----------------------------------------------------------
// ボスの向きと座標を更新、突進させる
//----------------------------------------------------------

void RushE::Update(float deltaTime)
{
	m_lifetime += deltaTime;

	//溜め（突進前）
	if (m_lifetime < CHARGE_TIME)
	{
		//プレイヤーの方向を向く
		float angle = atan2(m_forward.x, m_forward.z);
		m_enemy->SetRotationY(angle);
		m_position = m_enemy->GetPosition();

		if (m_particle)
		{
			//溜め時間
			float loopProgress = fmodf(m_lifetime * PREVIEW_PROGRESS_SPEED, 1.0f);
			//発生位置　未来の進む位置
			DirectX::SimpleMath::Vector3 spawnPos =
				m_position + m_forward * (PREVIEW_LINE_LENGTH * loopProgress);
			//足元より少し上に配置
			spawnPos.y += PREVIEW_SPAWN_HEIGHT_OFFSET;

			//パーティクルの生成
			m_particle->Spawn(
				Particle::Type::DashWarning,
				spawnPos,
				PREVIEW_PARTICLE_COUNT,		//粒子の数
				PREVIEW_PARTICLE_SIZE		//サイズ
			);
		}

	}
	//突進
	else
	{
		DirectX::SimpleMath::Vector3 currentPos = m_enemy->GetPosition();
		DirectX::SimpleMath::Vector3 nextPos = currentPos + m_forward * RUSH_SPEED * deltaTime;

		m_enemy->SetPosition(nextPos);
		m_position = nextPos;
	}

	if (m_collision)
	{
		m_collision->SetCenter(m_position);
	}
}

//----------------------------------------------------------
// 攻撃判定を描画
//----------------------------------------------------------
 
void RushE::Render(
	ID3D11DeviceContext* context,
	const DirectX::SimpleMath::Matrix& view,
	const DirectX::SimpleMath::Matrix& proj)
{
	UNREFERENCED_PARAMETER(context);
	UNREFERENCED_PARAMETER(view);
	UNREFERENCED_PARAMETER(proj);

	
}

//----------------------------------------------------------
// 当たり判定の範囲
//----------------------------------------------------------

DirectX::BoundingBox RushE::GetBoundingBox() const
{
	DirectX::BoundingBox box;
	if (m_collision)
	{
		box.Center = m_collision->GetCenter();
		box.Extents = m_collision->GetExtents();
	}
	else
	{
		// デフォルト小さい箱
		box.Center = m_position;
		box.Extents = DirectX::SimpleMath::Vector3(DEFAULT_BOUNDING_SIZE);
	}
	return box;
}
