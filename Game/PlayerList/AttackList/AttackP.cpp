//
// AttackP.cpp
//

#include "pch.h"
#include "AttackP.h"

#include <Effects.h>

using namespace DirectX;

using Microsoft::WRL::ComPtr;

//-----------------------------------------------------------------
// 近距離攻撃のインスタンスを生成
//-----------------------------------------------------------------

AttackP::AttackP(
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

	// モデル読み込み
	EffectFactory fx(device);
	fx.SetDirectory(L"Resources\\Models");
	m_attackModel = 
		Model::CreateFromSDKMESH(device,
			L"Resources\\Models\\Cube.sdkmesh",
			fx
	);

	SetForward(forward);

	m_forward = forward;
	m_forward.Normalize();


	//プレイヤーの前方に出す位置 
	m_position = playerPos + forward * SPAWN_OFFSET_FRONT;

	//OBB （幅 高さ 奥行き）
	if (m_attackModel)
    {
		//OBB初期化
        m_collision = std::make_unique<ModelCollisionOrientedBox>(m_attackModel.get());
		m_collision->SetCenter(m_position);
		m_collision->SetExtents(SimpleMath::Vector3(0.5f, 1.0f, 0.5f));
	}

	m_states = std::make_unique<CommonStates>(m_deviceResources->GetD3DDevice());
}

//-----------------------------------------------------------------
// 攻撃の更新処理
//-----------------------------------------------------------------

void AttackP::Update(float deltaTime)
{
	m_lifetime += deltaTime;

	// 弾の移動
	m_position += m_forward * 0.1f;

	if (m_collision)
	{
		m_collision->SetCenter(m_position);
	}
	// 寿命チェック
	if (m_lifetime >= MAX_LIFETIME)
	{
		//攻撃を無効化
		SetDead();   
		return;     
	}
}

//-----------------------------------------------------------------
// 攻撃のモデルやエフェクトを描画
//-----------------------------------------------------------------

void AttackP::Render(
	ID3D11DeviceContext* context,
	const DirectX::SimpleMath::Matrix& view,
	const DirectX::SimpleMath::Matrix& proj)
{
	if (!m_attackModel)return;

	SimpleMath::Matrix world = 
		SimpleMath::Matrix::CreateScale(1.0f) *
		SimpleMath::Matrix::CreateTranslation(m_position);

	
	//m_attackModel->Draw(context, *m_states, world, view, proj);

	//デバッグ描画 OBB → BoundingBox
	if (m_collision && m_displayCollision)
	{
		DirectX::BoundingBox box;
		box.Center = m_collision->GetCenter();
		box.Extents = m_collision->GetExtents();

		//コリジョン線
		m_displayCollision->AddBoundingBox(box, DirectX::Colors::White);
	}

	//攻撃の当たり判定を描画
	if (m_collision)
	{
		//攻撃オブジェクトのコリジョンを DisplayCollision に登録
		m_collision->AddDisplayCollision(m_displayCollision.get());

		//DisplayCollision に登録された情報を描画
		m_displayCollision->DrawCollision(
			context, m_states.get(), view, proj,
			DirectX::Colors::Red, DirectX::Colors::DarkRed, 0.5f
		);
	}

}

//-----------------------------------------------------------------
// 攻撃の判定がまだ残っているかを返す
//-----------------------------------------------------------------

bool AttackP::IsAlive() const
{
	return m_lifetime < MAX_LIFETIME;
}

//-----------------------------------------------------------------
// 攻撃の当たり判定を直接取得
//-----------------------------------------------------------------

DirectX::BoundingBox AttackP::GetBoundingBox() const
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
		box.Extents = DirectX::SimpleMath::Vector3(0.3f, 0.3f, 0.3f);
	}
	return box;
}
