
/**
 * @file   RushP.cpp
 * @brief  プレイヤーの突進攻撃を管理するクラスの実装
 * @author 國田知睦
 * @date   2026/07/01
 */

#include "pch.h"
#include "RushP.h"
#include <Effects.h>
#include "Game/PlayerList/Player.h"

//-----------------------------------------------------------------
// 突進攻撃のインスタンスを生成
//-----------------------------------------------------------------

RushP::RushP(
	DX::DeviceResources* deviceResources,
	Player* player, 
	const DirectX::SimpleMath::Vector3& forward,
	std::shared_ptr<DisplayCollision> displayCollision)
	:
	m_deviceResources(deviceResources),
	m_lifetime(0.0f),
	m_displayCollision(displayCollision),
	m_player(player)
{
	//コピー
	m_forward = forward;
	//水平に
	m_forward.y = 0.0f;
	
	//ベクトルの長さを確認
	if (m_forward.LengthSquared() < EPSILON)
	{
		m_forward = FORWARD;
	}
	m_forward.Normalize();

	//プレイヤーの座標を取得
	if (m_player)
	{
		m_position = m_player->GetPosition();
	}

	//当たり判定の作成
	m_collision = std::make_unique<ModelCollisionOrientedBox>();
	m_collision->SetCenter(m_position);
	m_collision->SetExtents(DEFAULT_BOX_SIZE);
 
	m_states = std::make_unique<DirectX::CommonStates>(m_deviceResources->GetD3DDevice());

	SetForward(m_forward);
}

//-----------------------------------------------------------------
// 攻撃の更新処理
//-----------------------------------------------------------------

void RushP::Update(float deltaTime)
{
	m_lifetime += deltaTime;

	//プレイヤー本体を移動
	DirectX::SimpleMath::Vector3 currentPos = m_player->GetPosition();
	//突進方向に移動
	DirectX::SimpleMath::Vector3 nextPos = currentPos;
	//突進速度を掛けて移動
	nextPos.x += m_forward.x * RUSH_SPEED * deltaTime;
	nextPos.z += m_forward.z * RUSH_SPEED * deltaTime;

	//プレイヤーの位置を更新
	m_player->SetPosition(nextPos);
	//攻撃判定もプレイヤーに追従
	m_position = nextPos;

	//当たり判定の中心を更新
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
	//ワールド座標の計算
	DirectX::SimpleMath::Matrix world =
		DirectX::SimpleMath::Matrix::CreateScale(SCALE_SIZE) *
		DirectX::SimpleMath::Matrix::CreateTranslation(m_position);

	//当たり判定
	if (m_collision&&m_displayCollision)
	{
		// 攻撃オブジェクトのコリジョンを DisplayCollision に登録
		m_collision->AddDisplayCollision(m_displayCollision.get());

		// DisplayCollision に登録された情報を描画（色は赤を設定）
		m_displayCollision->DrawCollision(
			context, m_states.get(), view, proj,
			DirectX::Colors::Red,
			DirectX::Colors::DarkRed, 
			DEBUG_COLLISION_LINE_THICKNESS
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
		box.Extents = DEFAULT_BOX_SIZE;
	}
	return box;
}
