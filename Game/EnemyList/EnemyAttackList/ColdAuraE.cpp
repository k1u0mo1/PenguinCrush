/**
 * @file   ColdAuraE.cpp
 * @brief  敵の範囲攻撃クラス
 * @author 國田知睦
 * @date   2026/10/22
 */

#include "pch.h"
#include "ColdAuraE.h"
#include "Game/EnemyList/BossEnemy.h"
#include "Game/EnemyList/EnemyBase.h"
#include <Effects.h>

//----------------------------------------------------------
// 敵の突進攻撃を生成
//----------------------------------------------------------

ColdAuraE::ColdAuraE(
	DX::DeviceResources* deviceResources,
	EnemyBase* enemy,
	const DirectX::SimpleMath::Vector3& forward,
	std::shared_ptr<DisplayCollision> displayCollision)
	: 
	m_deviceResources(deviceResources),
	m_lifetime(0.0f),
	m_isDead(false),
	m_displayCollision(displayCollision),
	m_enemy(enemy)
{
	UNREFERENCED_PARAMETER(forward);

	if (m_enemy)
	{
		m_position = m_enemy->GetPosition();
	}

	//球状の当たり判定を作成
	m_collision = std::make_unique<ModelCollisionSphere>();
	m_collision->SetCenter(m_position);
	m_collision->SetRadius(AURA_RADIUS);

	m_states = std::make_unique<DirectX::CommonStates>(m_deviceResources->GetD3DDevice());

	//冷気エフェクトの初期化
	m_smokeEffect = std::make_unique<Smoke>();
	m_smokeEffect->Initialize(m_deviceResources);

}

//----------------------------------------------------------
// ボスの向きと座標を更新、突進させる
//----------------------------------------------------------

void ColdAuraE::Update(float deltaTime)
{
	m_lifetime += deltaTime;
	
	//ボスの位置に常に追従させる
	if (m_enemy)
	{
		m_position = m_enemy->GetPosition();
	}

	if (m_collision)
	{
		m_collision->SetCenter(m_position);
	}
	
}

//----------------------------------------------------------
// 攻撃判定を描画
//----------------------------------------------------------
 
void ColdAuraE::Render(
	ID3D11DeviceContext* context,
	const DirectX::SimpleMath::Matrix& view,
	const DirectX::SimpleMath::Matrix& proj)
{
	if (m_collision && m_displayCollision)
	{
		m_collision->AddDisplayCollision(m_displayCollision.get());

		float progress = m_lifetime / MAX_LIFETIME;

		//発生からの時間に応じて色と透明度を変える
		DirectX::XMVECTOR drawColor;
		float alpha = 0.0f;

		//最初　予兆　赤色で警告
		if (m_lifetime < CHARGE_TIME)
		{
			//赤色の予兆
			drawColor = DirectX::Colors::Red;
			alpha = 0.25f * (m_lifetime / CHARGE_TIME);
		}
		//冷気的な範囲攻撃
		else
		{
			drawColor = DirectX::Colors::Cyan;
			//残り時間に応じて徐々に薄くしていく
			float auraProgress = (m_lifetime - CHARGE_TIME) / (MAX_LIFETIME - CHARGE_TIME);
			alpha = 0.6f * (1.0f - auraProgress);
		}

		//冷気煙
		if (m_smokeEffect)
		{
			DirectX::SimpleMath::Vector3 drawPos = m_position;
			drawPos.y += 0.1f;

			//範囲が一気に広がり、ゆっくり消える
			float currentScale = sinf(progress * DirectX::XM_PIDIV2) * AURA_RADIUS * 2.5f;
			float smokeAlpha = (1.0f - progress) * 0.8f;

			m_smokeEffect->Render(view, proj, drawPos, currentScale, smokeAlpha);
		}


		//デバックコリジョン計算
		m_displayCollision->DrawCollision(
			context, 
			m_states.get(),
			view,
			proj,
			drawColor,
			DirectX::Colors::Blue, 
			alpha
		);
	}
}

