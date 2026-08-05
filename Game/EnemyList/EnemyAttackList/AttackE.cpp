
/**
 * @file   Attack.cpp
 * @brief  敵の近距離攻撃クラス
 * @author 國田知睦
 * @date   2026/07/16
 */

#include "pch.h"
#include "AttackE.h"
#include <Effects.h>

//----------------------------------------------------------
// 短時間だけ発生する攻撃判定を生成
//----------------------------------------------------------

AttackE::AttackE(
	DX::DeviceResources* deviceResources,
	const DirectX::SimpleMath::Vector3& playerPos, 
	const DirectX::SimpleMath::Vector3& forward,
	std::shared_ptr<DisplayCollision> displayCollision)
	:
	m_deviceResources(deviceResources),
	m_forward(forward),
	m_lifetime(0.0f),
	m_displayCollision(displayCollision)
{
	auto device = m_deviceResources->GetD3DDevice();

	// モデル読み込み（Cube を流用）
	DirectX::EffectFactory fx(device);
	fx.SetDirectory(L"Resources\\Models");
	m_attackModel = 
		DirectX::Model::CreateFromSDKMESH(device,
			L"Resources\\Models\\Cube.sdkmesh",
			fx
	);

	//高さと距離調整
	DirectX::SimpleMath::Vector3 startPos = playerPos;

	//高さを上げる
	startPos.y += SPAWN_HEIGHT_OFFSET;
	//前方に出す
	m_position = startPos + forward * SPAWN_DISTANCE;

	//OBB （幅 高さ 奥行き）
	if (m_attackModel)
    {
		//OBB初期化
        m_collision = std::make_unique<ModelCollisionOrientedBox>(m_attackModel.get());
		m_collision->SetCenter(m_position);
		m_collision->SetExtents(COLLISION_SIZE);
	}

	m_states = std::make_unique<DirectX::CommonStates>(m_deviceResources->GetD3DDevice());
}

//----------------------------------------------------------
// 攻撃判定の移動と寿命を更新
//----------------------------------------------------------

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

//----------------------------------------------------------
// 攻撃判定を描画
//----------------------------------------------------------

void AttackE::Render(
	ID3D11DeviceContext* context,
	const DirectX::SimpleMath::Matrix& view,
	const DirectX::SimpleMath::Matrix& proj)
{
	UNREFERENCED_PARAMETER(context);
	UNREFERENCED_PARAMETER(view);
	UNREFERENCED_PARAMETER(proj);

	if (!m_attackModel)return;

	DirectX::SimpleMath::Matrix world =
		DirectX::SimpleMath::Matrix::CreateScale(SCALE_SIZE) *
		DirectX::SimpleMath::Matrix::CreateTranslation(m_position);

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

//----------------------------------------------------------
// 当たり判定の範囲
//----------------------------------------------------------

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
		box.Extents = 
			DirectX::SimpleMath::Vector3(
				DEFAULT_BOUNDING_SIZE
				);
	}
	return box;
}
