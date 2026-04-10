//
// RushP.cpp
//

#include "pch.h"
#include "RushP.h"

#include <Effects.h>

#include "Game/PlayerList/Player.h"

using namespace DirectX;

using Microsoft::WRL::ComPtr;

//-----------------------------------------------------------------
// 突進攻撃のインスタンスを生成
//-----------------------------------------------------------------

RushP::RushP(
	DX::DeviceResources* deviceResources,
	Player* player, 
	const DirectX::SimpleMath::Vector3& forward,
	std::shared_ptr<DisplayCollision> displayCollision)

	: m_deviceResources(deviceResources)
	//, m_forward(forward)
	, m_lifetime(0.0f)
	, m_displayCollision(displayCollision)
	, m_player(player)
{
	/*auto device = m_deviceResources->GetD3DDevice();*/

	//コピー
	m_forward = forward;
	//水平に
	m_forward.y = 0.0f;
	if (m_forward.LengthSquared() < 0.001f)
	{
		m_forward = DirectX::SimpleMath::Vector3(0.0f, 0.0f, 1.0f);
	}

	m_forward.Normalize();

	if (m_player)
	{
		m_position = m_player->GetPosition();
	}

	//当たり判定の作成
	m_collision = std::make_unique<ModelCollisionOrientedBox>();
	m_collision->SetCenter(m_position);
	m_collision->SetExtents(SimpleMath::Vector3(1.0f, 1.5f, 1.0f));

	//回転を進行方向に合わせる
	//float angle = atan2(m_forward.x, m_forward.z);

	m_states = std::make_unique<CommonStates>(m_deviceResources->GetD3DDevice());

	SetForward(m_forward);
}

//-----------------------------------------------------------------
// 攻撃の更新処理
//-----------------------------------------------------------------

void RushP::Update(float deltaTime)
{
	m_lifetime += deltaTime;

	//プレイヤー本体を移動
	float speed = 25.0f;
	SimpleMath::Vector3 currentPos = m_player->GetPosition();

	SimpleMath::Vector3 nextPos = currentPos;

	nextPos.x += m_forward.x * speed * deltaTime;
	nextPos.z += m_forward.z * speed * deltaTime;

	//プレイヤーの位置を更新
	m_player->SetPosition(nextPos);

	//攻撃判定もプレイヤーに追従
	m_position = nextPos;

	if (m_collision)
	{
		m_collision->SetCenter(m_position);
	}

}

//-----------------------------------------------------------------
// 攻撃のモデルやエフェクトを描画
//-----------------------------------------------------------------

void RushP::Render(
	ID3D11DeviceContext* context,
	const DirectX::SimpleMath::Matrix& view,
	const DirectX::SimpleMath::Matrix& proj)
{

	SimpleMath::Matrix world = 
		SimpleMath::Matrix::CreateScale(1.0f) *
		SimpleMath::Matrix::CreateTranslation(m_position);

	//当たり判定
	if (m_collision&&m_displayCollision)
	{
		// 攻撃オブジェクトのコリジョンを DisplayCollision に登録
		m_collision->AddDisplayCollision(m_displayCollision.get());

		// DisplayCollision に登録された情報を描画（色は赤を設定）
		m_displayCollision->DrawCollision(
			context, m_states.get(), view, proj,
			DirectX::Colors::Red, DirectX::Colors::DarkRed, 0.5f
		);
	}

}

//-----------------------------------------------------------------
// 攻撃が終了したか判断
//-----------------------------------------------------------------

bool RushP::IsDead() const
{
	return m_lifetime >= MAX_LIFETIME || m_isDead;
}

//-----------------------------------------------------------------
// 攻撃を強制的に無効化
//-----------------------------------------------------------------

void RushP::SetDead()
{
	m_isDead = true;
}

//-----------------------------------------------------------------
// 攻撃の当たり判定を直接取得
//-----------------------------------------------------------------

DirectX::BoundingBox RushP::GetBoundingBox() const
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
		box.Extents = DirectX::SimpleMath::Vector3(0.3f, 0.3f, 0.3f);
	}
	return box;
}
