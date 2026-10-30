
/**
 * @file   NormalEnemy.cpp
 * @brief  通常敵の管理を行うクラス
 * @author 國田知睦
 * @date   2026/10/25
 */

#include "pch.h"
#include "NormalEnemy.h"
#include "Game/EnemyList/EnemyManager.h"
#include "Game/SoundList/AudioManager.h"

//----------------------------------------------------------
// 通常敵のインスタンスを生成
//----------------------------------------------------------

NormalEnemy::NormalEnemy(
	DX::DeviceResources* deviceResources,
	const DirectX::SimpleMath::Vector3& position,
	const EnemyBaseParameter& param,
	std::shared_ptr<DisplayCollision> displayCollision)
	: 
	EnemyBase(deviceResources, position, param),
	m_deviceResources(deviceResources),
	m_displayCollision(displayCollision),
	m_stateTimer(0.0f)
{
	//攻撃パターンの追加
	m_attackPattern.push_back(std::make_shared<MeleeAttackPattern>());

	//個体差を出す
	float randomScale = ENEMY_SPEED +(static_cast<float>(rand()) / RAND_MAX) * SPEED_RAND_RANGE;
	m_param.speed *= randomScale;

	//プレイヤーとの距離をばらつかせる
	float randomOffset = ((static_cast<float>(rand()) / RAND_MAX)-0.5f) * STOP_DISTANCE_RAND_RANGE;
	m_stopDistance = STOP_DISTANCE + randomOffset;
}

//----------------------------------------------------------
// 通常敵の初期化
//----------------------------------------------------------

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
	AudioManager::GetInstance()->LoadSound("AttackE", L"Resources/Sounds/E_Close-Range_Attack.wav");
	AudioManager::GetInstance()->LoadSound("Fall", L"Resources/Sounds/SE_Falling_Water.wav");
}

//----------------------------------------------------------
// 通常敵の更新
//----------------------------------------------------------

void NormalEnemy::Update(
	float deltaTime,
	const DirectX::SimpleMath::Vector3& playerPosition,
	Stage* stage,
	Particle* particle,
	EnemyManager* enemyManager)
{

	if (IsDead())return;

	//タイマーの更新
	UpdateNormalEnemyTimers(deltaTime);
	//AI更新
	UpdateAI(deltaTime, playerPosition, stage, particle, enemyManager);
	//物理更新
	UpdatePhysics(stage, deltaTime,particle);
	//当たり判定の更新
	UpdateNormalEnemyCollision();
}

//----------------------------------------------------------
// 通常敵のAI更新
//----------------------------------------------------------

void NormalEnemy::UpdateAI(
	float dt,
	const DirectX::SimpleMath::Vector3& playerPos,
	Stage* stage,
	Particle* particle, 
	EnemyManager* enemyManager)
{
	UNREFERENCED_PARAMETER(particle);

	//死亡中はAIを止める
	if (m_state == EnemyState::Dying)
	{
		m_targetVelocity = DirectX::SimpleMath::Vector3::Zero;
		return;
	}

	float groundY = stage ? stage->GetGroundHeight(m_position.x, m_position.z) : 0.0f;
	if (m_position.y > groundY + 1.0f)
	{
		m_targetVelocity = DirectX::SimpleMath::Vector3::Zero;
		return;
	}

	//各攻撃パターンのクールダウンタイマーを更新
	for(auto& pattern : m_attackPattern)
	{
		pattern->UpdateTimer(dt);
	}

	//ノックバック中は移動しない
	if (m_knockbackTimer > 0.0f)
	{
		return;
	}
	
	//プレイヤーとの距離を計算
	float dist = DirectX::SimpleMath::Vector3::Distance(m_position, playerPos);
	DirectX::SimpleMath::Vector3 forward = playerPos - m_position;
	forward.y = 0.0f;
	
	//プレイヤーの方向を正規化
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
		//攻撃の判定
		if (enemyManager && !m_attackPattern.empty())
		{
			//プレイヤーとの距離が攻撃距離以内で、クールダウンが終わっている場合は攻撃
			for(auto& pattern : m_attackPattern)
			{
				//発動条件を満たしているか
				if (pattern->CanExecute(this, dist))
				{
					//攻撃を実行
					pattern->Execute(this, forward, enemyManager);

					//ステートと持続時間を切り替え
					m_state = pattern->GetState();
					m_stateTimer = pattern->GetDuration();
					break;
				}
			}
		}
		break;
	case EnemyState::Attack:
	default:
		break;
	}
}

//----------------------------------------------------------
// 通常敵の物理更新
//----------------------------------------------------------

void NormalEnemy::UpdatePhysics(Stage* stage, float dt,Particle* particle)
{
	//物理計算を更新
	CharacterBase::UpdatePhysice(dt, stage, particle);

	//滑る床の挙動を管理するクラスを更新
	if (stage)
	{
		//滑る床の方向を取得
		DirectX::SimpleMath::Vector3 slideDir = stage->GetSlideDirection(m_position.x, m_position.z);
		//滑る床の挙動を更新
		m_slideBehavior.Update(m_position, m_targetVelocity, slideDir, dt);
	}
}

//----------------------------------------------------------
// 通常の敵のタイマー更新
//----------------------------------------------------------

void NormalEnemy::UpdateNormalEnemyTimers(float deltaTime)
{
	//死亡中ならタイマーを加算
	if (m_state == EnemyState::Dying)
	{
		m_dyingTimer += deltaTime;
		return;
	}

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
}

//----------------------------------------------------------
// 通常の敵の当たり判定の更新
//----------------------------------------------------------

void NormalEnemy::UpdateNormalEnemyCollision()
{
	//当たり判定の更新
	if (!m_modelCollision)return;
	
	//モデルのスケールと位置を反映させたワールド行列を作成
	DirectX::SimpleMath::Matrix world =
		DirectX::SimpleMath::Matrix::CreateScale(m_param.scale) *
		DirectX::SimpleMath::Matrix::CreateTranslation(m_position);
	//モデルの当たり判定を更新
	m_modelCollision->UpdateBoundingInfo(world);
	
}

//----------------------------------------------------------
// 通常敵の描画
//----------------------------------------------------------

void NormalEnemy::Render(
	ID3D11DeviceContext* context,
	const DirectX::SimpleMath::Matrix& view,
	const DirectX::SimpleMath::Matrix& proj)
{
	if (IsDead())return;
	//モデルの回転行列を作成
	DirectX::SimpleMath::Matrix rotX = DirectX::SimpleMath::Matrix::Identity;
	DirectX::SimpleMath::Matrix rotation =
		DirectX::SimpleMath::Matrix::CreateRotationY(m_rotationY + DirectX::XM_PI);
	DirectX::SimpleMath::Matrix combinedTransform =
		DirectX::SimpleMath::Matrix::CreateScale(m_param.scale) *
		rotX * rotation;

	if (m_renderer)
	{
		//Dying状態ならm_dyingTimerを渡す
		float timerParam =
			(m_state == EnemyState::Dying) ? m_dyingTimer : 0.0f;

		//モデルの描画
		m_renderer->Render(
			context,
			m_position, 
			m_forward,
			m_state, 
			timerParam,
			combinedTransform,
			view, 
			proj,
			nullptr,
			nullptr
		);
	}
	//当たり判定の描画
	if (m_displayCollision && m_states)
	{
		m_displayCollision->DrawCollision(
			context, m_states.get(), view, proj,
			DirectX::Colors::White, DirectX::Colors::Blue, COLLISION_DRAW_ALPHA_NORMAL
		);
	}
	//モデルの当たり判定の描画
	if (m_modelCollision && m_displayCollision && m_states)
	{
		m_modelCollision->AddDisplayCollision(m_displayCollision.get());
		m_displayCollision->DrawCollision(
			context, m_states.get(), view, proj,
			DirectX::Colors::White, DirectX::Colors::Cyan, COLLISION_DRAW_ALPHA_ACTIVE
		);
	}
}

//----------------------------------------------------------
// 通常敵のダメージ処理
//----------------------------------------------------------

void NormalEnemy::TakeDamage(
	float amount, PlayerAttackType type)
{
	UNREFERENCED_PARAMETER(type);

	if (IsDead() || m_state == EnemyState::Dying) return;

	//体力を減らす
	m_hp -= amount;
	if (m_hp <= 0) 
	{
		m_hp = 0;
		//死亡演出に
		m_state = EnemyState::Dying;
		m_dyingTimer = 0.0f;
	}
}

//----------------------------------------------------------
// 通常敵のノックバック処理
//----------------------------------------------------------

void NormalEnemy::ApplyKnockback(
	const DirectX::SimpleMath::Vector3& direction,
	float power)
{
	
	CharacterBase::ApplyKnockback(
		direction,
		power,
		m_param.receivedKnockbackPowerMultiplier,
		m_param.receivedKnockbackUpwardForce,
		m_param.receivedKnockbackDuration
	);
}