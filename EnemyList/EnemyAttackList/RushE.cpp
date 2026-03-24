//
// RushE.cpp
//

#include "pch.h"
#include "RushE.h"

#include "EnemyList/BossEnemy.h"

#include <Effects.h>

using namespace DirectX;

using Microsoft::WRL::ComPtr;

RushE::RushE(
	DX::DeviceResources* deviceResources,
	/*const DirectX::SimpleMath::Vector3& playerPos, */
	BossEnemy* boss,
	const DirectX::SimpleMath::Vector3& forward,
	std::shared_ptr<DisplayCollision> displayCollision)
	: m_deviceResources(deviceResources)
	, m_forward(forward)
	, m_lifetime(0.0f)
	, m_displayCollision(displayCollision)
	, m_boss(boss)
{
	

	
	//ボスの位置に当たり判定を作る
	m_position = m_boss->GetPosition();

	//当たり判定
	m_collision = std::make_unique<ModelCollisionOrientedBox>();
	m_collision->SetCenter(m_position);
	m_collision->SetExtents(SimpleMath::Vector3(1.0f, 1.5f, 1.0f));

	
	////OBB （幅 高さ 奥行き）
	//if (m_rushModel)
 //   {
	//	//OBB初期化
 //       m_collision = std::make_unique<ModelCollisionOrientedBox>(m_rushModel.get());
	//	m_collision->SetCenter(m_position);
	//	m_collision->SetExtents(SimpleMath::Vector3(0.5f, 1.0f, 0.5f));
	//}

	m_states = std::make_unique<CommonStates>(m_deviceResources->GetD3DDevice());
}



void RushE::Update(float deltaTime)
{
	m_lifetime += deltaTime;

	
	//ボスの向きを進行方向に向ける
	float angle = atan2(m_forward.x, m_forward.z);
	m_boss->SetRotationY(angle);

	//ボスを滑らせる(移動)
	SimpleMath::Vector3 currentPos = m_boss->GetPosition();
	
	SimpleMath::Vector3 nextPos = currentPos + m_forward * RUSH_SPEED * deltaTime;

	//ボスの位置を更新
	m_boss->SetPosition(nextPos);

	//当たり判定もボスに追従
	m_position = nextPos;

	if (m_collision)
	{
		m_collision->SetCenter(m_position);
	}

}

void RushE::Render(
	ID3D11DeviceContext* context,
	const DirectX::SimpleMath::Matrix& view,
	const DirectX::SimpleMath::Matrix& proj)
{


	SimpleMath::Matrix world = 
		SimpleMath::Matrix::CreateScale(1.0f) *
		SimpleMath::Matrix::CreateTranslation(m_position);

	
	//m_attackModel->Draw(context, *m_states, world, view, proj);

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
			Colors::Red, Colors::DarkRed, 0.5f
		);
	}

}

//bool RushE::IsDead() const
//{
//	return m_lifetime < MAX_LIFETIME;
//}

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
		box.Extents = DirectX::SimpleMath::Vector3(DEFAULT_BOUNDING_SIZE, DEFAULT_BOUNDING_SIZE, DEFAULT_BOUNDING_SIZE);
	}
	return box;
}
