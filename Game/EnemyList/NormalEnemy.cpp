
/**
 * @file   NormalEnemy.cpp
 * @brief  通常敵の管理を行うクラス
 * @author 國田知睦
 * @date   2026/06/24
 */

#include "pch.h"
#include "NormalEnemy.h"
#include "Game/EnemyList/EnemyManager.h"
#include "Game/SoundList/AudioManager.h"

using namespace DirectX;


NormalEnemy::NormalEnemy(
	DX::DeviceResources* deviceResources,
	const DirectX::SimpleMath::Vector3& position,
	const EnemyBaseParameter& param,
	std::shared_ptr<DisplayCollision> displayCollision)
	: EnemyBase(deviceResources, position, param)
	, m_deviceResources(deviceResources)
	, m_displayCollision(displayCollision)
	, m_state(EnemyState::Idle)
	, m_stateTimer(0.0f)
{
	//攻撃パターンの追加
	m_attackPattern.push_back(std::make_shared<MeleeAttackPattern>());
	m_meleeAttackCooldownTimer = 0.0f;

	//個体差を出す
	float randomScale = ENEMY_SPEED +(static_cast<float>(rand()) / RAND_MAX) * SPEED_RAND_RANGE;
	m_param.speed *= randomScale;

	//プレイヤーとの距離をばらつかせる
	float randomOffset = ((static_cast<float>(rand()) / RAND_MAX)-0.5f) * STOP_DISTANCE_RAND_RANGE;
	m_stopDistance = STOP_DISTANCE + randomOffset;

}

void NormalEnemy::Initialize()
{
	auto device = m_deviceResources->GetD3DDevice();

	//当たり判定のモデルの初期化
	m_renderer = std::make_unique<EnemyRenderer>();
	m_renderer->Initialize(device);

	//当たり判定のモデルを生成
	if(m_renderer->GetMainModel())
	{
		m_modelCollision = std::make_unique<ModelCollisionOrientedBox>(m_renderer->GetMainModel());
	}

	//当たり判定の表示用のポインタにモデルをセット
	m_states = std::make_unique< DirectX::DX11::CommonStates>(device);

	//サウンドの読み込み
	AudioManager::GetInstance()->LoadSound("AttackE", L"Resources/Sounds/E_近距離攻撃.wav");
	AudioManager::GetInstance()->LoadSound("Fall", L"Resources/Sounds/P_E_落水.wav");
}

void NormalEnemy::Update(
	float deltaTime,
	const DirectX::SimpleMath::Vector3& playerPosition,
	Stage* stage,
	Particle* particle,
	EnemyManager* enemyManager)
{
	if (IsDead())return;

	//物理更新
	if (m_stateTimer > 0.0f)
	{
		m_stateTimer -= deltaTime;
		//状態の更新
		if (m_stateTimer <= 0.0f)
		{
			m_state = EnemyState::Idle;
		}
	}

	//AI更新
	UpdateAI(deltaTime, playerPosition, stage, particle, enemyManager);

	//物理更新
	UpdatePhysics(stage, deltaTime);

	//当たり判定の更新
	if (m_modelCollision)
	{
		//モデルのスケールと位置を反映させたワールド行列を作成
		SimpleMath::Matrix world = 
			SimpleMath::Matrix::CreateScale(m_param.scale) *
			SimpleMath::Matrix::CreateTranslation(m_position);

		//モデルの当たり判定を更新
		m_modelCollision->UpdateBoundingInfo(world);
	}


}

void NormalEnemy::UpdateAI(
	float dt,
	const DirectX::SimpleMath::Vector3& playerPos,
	Stage* stage,
	Particle* /*particle*/, 
	EnemyManager* enemyManager)
{
	if (m_meleeAttackCooldownTimer > 0.0f)
	{
		m_meleeAttackCooldownTimer -= dt;
	}

	//ノックバック中は移動しない
	if (m_knockbackTimer > 0.0f)
	{
		return;
	}

	
	m_targetVelocity = DirectX::SimpleMath::Vector3::Zero;

	//プレイヤーとの距離を計算
	float dist = DirectX::SimpleMath::Vector3::Distance(m_position, playerPos);
	DirectX::SimpleMath::Vector3 forward = playerPos - m_position;
	forward.y = 0.0f;

	
	if (forward.LengthSquared() > VECTOR_EPSILON)
	{
		forward.Normalize();
	}

	//目標速度を初期化
	m_targetVelocity = DirectX::SimpleMath::Vector3::Zero;
	

	switch (m_state)
	{
	case EnemyState::Idle:
		//プレイヤーとの距離
		if (dist > m_stopDistance)
		{
			if (m_enemyAI)
			{
				m_enemyAI->UpdateAI(
					dt,
					m_position,
					playerPos,
					stage,
					m_param.speed,
					m_targetVelocity,
					m_rotationY
				);

			}
		}
		else
		{
			//プレイヤーに近づきすぎないようにする
			if (forward.LengthSquared() > VECTOR_EPSILON)
			{
				m_rotationY = std::atan2(forward.x, forward.z);
			}
		}

		if (enemyManager && !m_attackPattern.empty())
		{
			//
			if (dist < ATTACK_DISTANCE && m_meleeAttackCooldownTimer <= 0.0f)
			{
				m_attackPattern[0]->Execute(this, forward, enemyManager);

				m_state = EnemyState::Attack;
				m_stateTimer = ATTACK_STATE_DURATION;
				m_meleeAttackCooldownTimer = ATTACK_COOLDOWN;
			}
		}

		break;
	case EnemyState::Attack:
	default:
		break;
	
	}

}

void NormalEnemy::UpdatePhysics(Stage* stage, float dt,Particle* particle)
{
	//滑る床の挙動を更新
	CharacterBase::UpdatePhysice(dt, stage);

	CheckAndHandleFalling(stage, particle);

	//滑る床の挙動を管理するクラスを更新
	if (stage)
	{
		//滑る床の方向を取得
		SimpleMath::Vector3 slideDir = stage->GetSlideDirection(m_position.x, m_position.z);
		//滑る床の挙動を更新
		m_slideBehavior.Update(m_position, m_targetVelocity, slideDir, dt);

		//落下の判定と処理
		CheckAndHandleFalling(stage, nullptr);
	}

}

void NormalEnemy::Render(
	ID3D11DeviceContext* context,
	const DirectX::SimpleMath::Matrix& view,
	const DirectX::SimpleMath::Matrix& proj)
{
	if (IsDead())return;

	SimpleMath::Matrix rotX = SimpleMath::Matrix::Identity;
	
	SimpleMath::Matrix rotation =
		SimpleMath::Matrix::CreateRotationY(m_rotationY + DirectX::XM_PI);
	
	SimpleMath::Matrix combinedTransform =
		SimpleMath::Matrix::CreateScale(m_param.scale) *
		rotX * rotation;

	if (m_renderer)
	{
		//モデルの描画
		m_renderer->Render(
			context, m_position, m_forward,m_state, 0.0f,
			combinedTransform, view, proj, nullptr, nullptr
		);
	}

	if (m_displayCollision && m_states)
	{
		m_displayCollision->DrawCollision(
			context, m_states.get(), view, proj,
			Colors::White, Colors::Blue, COLLISION_DRAW_ALPHA_NORMAL
		);
	}

	if (m_modelCollision && m_displayCollision && m_states)
	{
		m_modelCollision->AddDisplayCollision(m_displayCollision.get());
		m_displayCollision->DrawCollision(
			context, m_states.get(), view, proj,
			Colors::White, Colors::Cyan, COLLISION_DRAW_ALPHA_ACTIVE
		);
	}
}

void NormalEnemy::TakeDamage(
	float amount, PlayerAttackType /*type*/)
{
	if (IsDead())return;

	m_hp -= amount;
	if (m_hp < 0)m_hp = 0;
	
}

void NormalEnemy::ApplyKnockback(
	const DirectX::SimpleMath::Vector3& direction,
	float power)
{
	//敵自身が受けるノックバックの倍率
	//後ろに飛ぶ強さ
	m_knockbackVelocity = direction * power * m_param.receivedKnockbackPowerMultiplier;
	//上に飛ぶ強さ
	m_knockbackVelocity.y = m_param.receivedKnockbackUpwardForce;
	//ノックバック後の硬直
	m_knockbackTimer = m_param.receivedKnockbackDuration;

}