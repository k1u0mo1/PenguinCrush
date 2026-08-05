
/**
 * @file   EnemyBase.cpp
 * @brief  敵の管理を行う基底クラス
 * @author 國田知睦
 * @date   2026/07/01
 */

#include "pch.h"
#include "EnemyBase.h"
#include "Game/Collision/ModelCollision.h"
#include "Game/SoundList/AudioManager.h"
#include "Game/GimmickList/Stage.h"
#include "Game/Effects/Particle.h"

 //----------------------------------------------------------
 // 敵のコンストラクタ
 //----------------------------------------------------------

EnemyBase::EnemyBase(DX::DeviceResources* deviceResources,
	const DirectX::SimpleMath::Vector3& position,
	const EnemyBaseParameter& param)
	: 
	CharacterBase(),
	m_param(param),
	m_deviceResources(deviceResources) ,
	m_rotationY(0.0f)
{
	m_position = position;
	m_hp = param.maxHp;
	m_maxHp = param.maxHp;
	m_speed = param.speed;

	//AIのインスタンスを生成
	m_enemyAI = std::make_unique<EnemyAI>();
}

//----------------------------------------------------------
// 敵の初期化
//----------------------------------------------------------

void EnemyBase::Initialize()
{
}

//----------------------------------------------------------
// 敵の更新
//----------------------------------------------------------

void EnemyBase::Update(
	float dt,
	const DirectX::SimpleMath::Vector3& playerPos,
	Stage* stage,
	Particle* particle,
	EnemyManager* enemyManager
)
{
	//AIの思考処理
	UpdateAI(dt, playerPos, stage, particle, enemyManager);
	//物理演算と移動処理
	UpdatePhysice(dt, stage);
}

//----------------------------------------------------------
// 敵の落下の判定と処理
//----------------------------------------------------------

void EnemyBase::CheckAndHandleFalling(
	Stage* stage,
	Particle* particle)
{
	//落下の判定
	if (m_position.y < FALL_LIMIT_Y)
	{
		if (particle)
		{
			//落ちたときの水しぶきのエフェクトを発生させる
			DirectX::SimpleMath::Vector3 splashPosition = m_position;
			splashPosition.y = 0.0f;
			//水しぶきのエフェクトを発生させる
			particle->Spawn(Particle::Type::Splash, splashPosition, SPLASH_PARTICLE_COUNT);
		}

		//安全なスポットを探す
		bool foundSafeSpot = false;
		DirectX::SimpleMath::Vector3 safePos;

		//ステージの範囲内を○m間隔でチェックしていく
		for(float checkX=-CHECK_RANDE; checkX<= CHECK_RANDE; checkX+= CHECK_INTERVAL)
		{
			//ステージの範囲内を○m間隔でチェックしていく
			for (float checkZ = -CHECK_RANDE; checkZ <= CHECK_RANDE; checkZ += CHECK_INTERVAL)
			{
				//チェックしているスポットの地面の高さを取得
				float groundY = stage->GetGroundHeight(checkX, checkZ);
				if (groundY > STAGE_BOUNDARY_Y)
				{
					//安全なスポットを見つけたので、そこに復活させる
					safePos = DirectX::SimpleMath::Vector3(checkX, RESPAWN_HEIGHT, checkZ);
					//落下している場合はリスポーン位置に移動させる
					foundSafeSpot = true;
					break;
				}
			}
		}
		//安全な場所
		if (foundSafeSpot)
		{
			//座標を合わせる
			m_position = safePos;
		}
		else
		{
			m_hp = 0.0f;
		}

		m_velocity = DirectX::SimpleMath::Vector3::Zero;

		m_hp -= FALL_DAMAGE;

		if (m_hp < 0)
		{
			m_hp = 0.0f;
		}

		AudioManager::GetInstance()->Play("Fall");
	}
}
