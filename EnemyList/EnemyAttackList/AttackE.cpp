//
// AttackE.cpp
//

#include "pch.h"
#include "AttackE.h"

#include <Effects.h>

using namespace DirectX;

using Microsoft::WRL::ComPtr;

AttackE::AttackE(
	DX::DeviceResources* deviceResources,
	const DirectX::SimpleMath::Vector3& playerPos, 
	const DirectX::SimpleMath::Vector3& forward,
	std::shared_ptr<DisplayCollision> displayCollision)

	: m_deviceResources(deviceResources)
	, m_forward(forward)
	, m_lifetime(0.0f)
	, m_displayCollision(displayCollision)
{
	auto device = m_deviceResources->GetD3DDevice();

	// モデル読み込み（Cube を流用）
	EffectFactory fx(device);
	fx.SetDirectory(L"Resources\\Models");
	m_attackModel = 
		Model::CreateFromSDKMESH(device,
			L"Resources\\Models\\Cube.sdkmesh",
			fx
	);

	//高さと距離調整
	DirectX::SimpleMath::Vector3 startPos = playerPos;

	//高さを上げる
	startPos.y += SPAWN_HEIGHT_OFFSET;

	m_position = startPos + forward * SPAWN_DISTANCE;


	//OBB （幅 高さ 奥行き）
	if (m_attackModel)
    {
		//OBB初期化
        m_collision = std::make_unique<ModelCollisionOrientedBox>(m_attackModel.get());
		m_collision->SetCenter(m_position);
		m_collision->SetExtents(SimpleMath::Vector3(1.0f, 1.0f, 1.0f));
	}

	m_states = std::make_unique<CommonStates>(m_deviceResources->GetD3DDevice());

	
}



void AttackE::Update(float deltaTime)
{
	m_lifetime += deltaTime;

	// 弾の移動
	m_position += m_forward * deltaTime * ATTACK_SPEED;

	if (m_collision)
	{
		m_collision->SetCenter(m_position);
	}

}

void AttackE::Render(
	ID3D11DeviceContext* /*context*/,
	const DirectX::SimpleMath::Matrix& /*view*/,
	const DirectX::SimpleMath::Matrix& /*proj*/)
{


	if (!m_attackModel)return;

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

		//コリジョン線
		m_displayCollision->AddBoundingBox(box, DirectX::Colors::White);
	}

}

//bool AttackE::IsDead() const
//{
//	return m_lifetime < MAX_LIFETIME;
//}

DirectX::BoundingBox AttackE::GetBoundingBox() const
{
	DirectX::BoundingBox box;
	if (m_collision)
	{
		box.Center = m_collision->GetCenter();
		box.Extents = m_collision->GetExtents();
	}
	else
	{
		//デフォルト小さい箱
		box.Center = m_position;
		box.Extents = DirectX::SimpleMath::Vector3(DEFAULT_BOUNDING_SIZE, DEFAULT_BOUNDING_SIZE, DEFAULT_BOUNDING_SIZE);
	}
	return box;
}
