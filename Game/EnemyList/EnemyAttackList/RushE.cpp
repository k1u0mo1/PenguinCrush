/**
 * @file   RushE.cpp
 * @brief  敵の突進攻撃クラス
 * @author 國田知睦
 * @date   2026/07/01
 */

#include "pch.h"
#include "RushE.h"
#include "Game/EnemyList/BossEnemy.h"
#include "Game/EnemyList/EnemyBase.h"
#include <Effects.h>

//----------------------------------------------------------
// 敵の突進攻撃を生成
//----------------------------------------------------------

RushE::RushE(
	DX::DeviceResources* deviceResources,
	EnemyBase* enemy,
	const DirectX::SimpleMath::Vector3& forward,
	std::shared_ptr<DisplayCollision> displayCollision)
	: 
	m_deviceResources(deviceResources),
	m_forward(forward),
	m_lifetime(0.0f),
	m_displayCollision(displayCollision),
	m_enemy(enemy)
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
	
	//敵の向きをプレイヤーの方に向ける
	float angle = atan2(m_forward.x, m_forward.z);
	m_enemy->SetRotationY(angle);

	//ボスを滑らせる(移動)
	DirectX::SimpleMath::Vector3 currentPos = m_enemy->GetPosition();
	//ボスの前方に出す
	DirectX::SimpleMath::Vector3 nextPos = currentPos + m_forward * RUSH_SPEED * deltaTime;

	//ボスの位置を更新
	m_enemy->SetPosition(nextPos);

	//当たり判定もボスに追従
	m_position = nextPos;
	//コリジョン
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
	//合成
	DirectX::SimpleMath::Matrix world =
		DirectX::SimpleMath::Matrix::CreateScale(SCALE_SIZE) *
		DirectX::SimpleMath::Matrix::CreateTranslation(m_position);

	// デバッグ描画 OBB → BoundingBox
	if (m_collision && m_displayCollision)
	{
		DirectX::BoundingBox box;
		box.Center = m_collision->GetCenter();
		box.Extents = m_collision->GetExtents();

		m_displayCollision->AddBoundingBox(box, DirectX::Colors::White);
	}

	// デバッグ用の当たり判定表示だけ残す
	if (m_collision && m_displayCollision)
	{
		m_collision->AddDisplayCollision(m_displayCollision.get());
		// 赤色で攻撃判定を表示
		m_displayCollision->DrawCollision(
			context, m_states.get(), view, proj,
			DirectX::Colors::Red, DirectX::Colors::DarkRed, DEBUG_COLLISION_LINE_THICKNESS
		);
	}
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
