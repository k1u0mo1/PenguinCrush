
/**
 * @file   BossEnemy.cpp
 * @brief  ボスの管理を行うクラス
 * @author 國田知睦
 * @date   2026/07/01
 */

#include "pch.h"
#include "BossEnemy.h"
#include <Effects.h>
#include <Game/SoundList/AudioManager.h>
#include <Game/Effects/Particle.h>
#include <Game/Effects/Smoke.h>
#include "Game/EnemyList/EnemyRenderer.h"
#include "Game/EnemyList/EnemyManager.h"
#include "Game/EnemyList/EnemyAttackList/EnemyAttackPattern.h"
#include "Game/EnemyList/EnemyBaseParameter.h"

//----------------------------------------------------------
// ボスのインスタンスを生成
//----------------------------------------------------------

BossEnemy::BossEnemy(
    DX::DeviceResources* deviceResources,
    const DirectX::SimpleMath::Vector3& position,
    std::shared_ptr<DisplayCollision> displayCollision,
    EnemyType type
)
    : 
    EnemyBase(deviceResources,position,EnemyData::BossEnemy),
    m_deviceResources(deviceResources),
    m_displayCollision(displayCollision),
    m_isSmokeActive(false),
    m_isLandingEffectDone(false),
    m_smokeTimer(0.0f),
    m_isGroundPrev(false),
    m_isGroundNow(false)
{
    m_type = type;

	//攻撃パターンの追加
    m_attackPatterns.push_back(std::make_shared<MeleeAttackPattern>());
	m_attackPatterns.push_back(std::make_shared<RushAttackPattern>());

    m_meleeTimer = 0.0f;
    m_rushTimer = 0.0f;
}

//----------------------------------------------------------
// デストラクタ
//----------------------------------------------------------

BossEnemy::~BossEnemy()
{
}

//----------------------------------------------------------
// モデルやエフェクト、サウンドなどの初期化処理
//----------------------------------------------------------

void BossEnemy::Initialize()
{
    auto device = m_deviceResources->GetD3DDevice();

    //敵の描画初期化
    m_renderer = std::make_unique<EnemyRenderer>();
	m_renderer->Initialize(device);


    if (m_renderer->GetMainModel())
    {
        m_collision = std::make_unique<ModelCollisionOrientedBox>(m_renderer->GetMainModel());
    }

    m_states = std::make_unique<DirectX::CommonStates>(device);

    //音の読み込み
    AudioManager::GetInstance()->LoadSound("AttackE", L"Resources/Sounds/E_近距離攻撃.wav");
    AudioManager::GetInstance()->LoadSound("DashE", L"Resources/Sounds/E_突進攻撃.wav");
    AudioManager::GetInstance()->LoadSound("Fall", L"Resources/Sounds/P_E_落水.wav");

    //エフェクト系
    //煙の生成と初期化
    m_smokeEffect = std::make_unique<Smoke>();
    m_smokeEffect->Initialize(m_deviceResources);

    m_isSmokeActive = false;
    m_smokeTimer = 0.0f;

    //着地している
    m_isGroundPrev = true;
}

//----------------------------------------------------------
// ボスのAI、物理挙動、状態遷移を更新
//----------------------------------------------------------

void BossEnemy::Update(float deltaTime,
    const DirectX::SimpleMath::Vector3& playerPosition,
    Stage* stage,
    Particle* particle,
	EnemyManager* enemyManager
)
{
    if (IsDead()) return;

    //------------------------------------------------
    //演出のタイマーが動いていれば減らす
    //------------------------------------------------
    if (m_stateTimer > 0.0f)
    {
        m_stateTimer -= deltaTime;
        if (m_stateTimer <= 0.0f)
        {
            //止まるー＞通常にする
            m_state = EnemyState::Idle;
        }
    }
    
    //------------------------------------------------
    //AIによる思考処理
    //------------------------------------------------
    UpdateAI(deltaTime, playerPosition, stage,particle,enemyManager);

    //------------------------------------------------
    //物理演算と移動処理
    //------------------------------------------------
    UpdatePhysics(stage, deltaTime);
   
    //------------------------------------------------
    //エフェクト処理
    //------------------------------------------------

    //着地判定
    m_isGroundNow = (m_position.y <= GROUND_LEVEL_THRESHOLD);

    //空中ー＞着地
    if (m_isGroundNow && !m_isGroundPrev&&!m_isLandingEffectDone)
    {
        //エフェクト
        m_isSmokeActive = true;

        m_smokeTimer = 0.0f;

        m_isLandingEffectDone = true;
    }
    //今の状態を覚える
    m_isGroundPrev = m_isGroundNow;

    //煙の更新
    if (m_isSmokeActive)
    {
        m_smokeTimer += deltaTime;

        //経過で消えるように
        if (m_smokeTimer >= SMOKE_DURATION)
        {
            m_isSmokeActive = false;
        }
    }
    
    //------------------------------------------------
    //当たり判定の座標更新
    //------------------------------------------------
    // OBB 更新
    if (m_collision)
    {
        DirectX::SimpleMath::Matrix world =
            DirectX::SimpleMath::Matrix::CreateScale(m_param.scale)
            * DirectX::SimpleMath::Matrix::CreateTranslation(m_position);

        m_collision->UpdateBoundingInfo(world);
    }
}

//----------------------------------------------------------
// ボスのモデルやエフェクト、デバッグ用コリジョンを描画
//----------------------------------------------------------

void BossEnemy::Render(
    ID3D11DeviceContext* context,
    const DirectX::SimpleMath::Matrix& view,
    const DirectX::SimpleMath::Matrix& proj
    )
{
    if (IsDead()) return;
   
	//モデルの回転行列を作る前に、X軸の回転行列を初期化
    DirectX::SimpleMath::Matrix rotX = DirectX::SimpleMath::Matrix::Identity;
    // Y軸の回転行列 (計算した角度)
    DirectX::SimpleMath::Matrix rotation =
        DirectX::SimpleMath::Matrix::CreateRotationY(m_rotationY+DirectX::XM_PI);
	//モデルの回転と拡大を組み合わせた行列
    DirectX::SimpleMath::Matrix combinedTransform =
        DirectX::SimpleMath::Matrix::CreateScale(m_param.scale) *
        rotX * 
		rotation;
    
	//モデルの描画
    if (m_renderer)
    {
        m_renderer->Render(
            context,
            m_position,
            m_forward,
            m_state,
            0.0f,
            combinedTransform,
            view,
            proj,
            nullptr, 
            nullptr  
        );
    }
    //コリジョン描画
    m_displayCollision->DrawCollision(
        context, m_states.get(), view, proj,
        DirectX::Colors::White, DirectX::Colors::Blue, TRANSPARENCY
    );

    //煙の描画
    if (m_isSmokeActive)
    {
        //時間経過に合わせて大きくする 
        float scale = SMOKE_SCALE + m_smokeTimer * SMOKE_GROWTH_RATE;
        //時間経過に合わせて透明にする 
        float alpha = 1.0f - (m_smokeTimer / SMOKE_DURATION);
        //煙の描画実行
        DirectX::SimpleMath::Vector3 drawPos = m_position;
        drawPos.y += SMOKE_Y_OFFSET;
		//煙の描画実行
        m_smokeEffect->Render(view, proj, drawPos, scale, alpha);
    }

    if (m_collision)
    {
        //敵のコリジョンを描画 
        m_collision->AddDisplayCollision(m_displayCollision.get()); 

        m_displayCollision->DrawCollision(
            context, m_states.get(), view, proj,
            DirectX::Colors::White, DirectX::Colors::Blue, 0.5f // 敵は青色で表示
        );
    }
}

//----------------------------------------------------------
// ボスにダメージを与え、連続攻撃による回避判定
//----------------------------------------------------------

void BossEnemy::TakeDamage(float amount, PlayerAttackType type)
{
    //同じ攻撃を連続で食らった場合
    if (type == m_lastAttackType)
    {
        m_consecutiveHitCount++;
    }
    else
    {
        //違ったらカウントを戻す
        m_consecutiveHitCount = EVADE_NOHIT_THRESHOLD;
        //最後に受けた攻撃のタイプ
        m_lastAttackType = type;
    }

    //回避発動 3回連続で同じ攻撃だったら
    if (m_consecutiveHitCount >= EVADE_HIT_THRESHOLD)
    {
        //回避状態へ
        SetState(EnemyState::Avoid, EVADE_DURATION);
        return;
    }

	//気絶状態でなければ攻撃回数をカウント
    if (m_state != EnemyState::Dizzy)
    {
        m_playerAttackCounter++;

        if (m_playerAttackCounter >= STUN_HIT_THRESHOLD)
        {
            m_playerAttackCounter = 0;
			//気絶状態へ　何秒動けないか
            SetState(EnemyState::Dizzy, STUN_TIME);
        }
    }

    //HPを減らす
    m_hp -= amount;

    if (m_hp < 0) m_hp = 0;
}

//----------------------------------------------------------
// 攻撃を受けた際のノックバック力をボスに適用
//----------------------------------------------------------

void BossEnemy::ApplyKnockback(const DirectX::SimpleMath::Vector3& direction, float power)
{
    //敵自身が受けるノックバックの倍率
    //後ろに飛ぶ強さ
    m_knockbackVelocity = direction * power * m_param.receivedKnockbackPowerMultiplier;
    //上に飛ぶ強さ
    m_knockbackVelocity.y = m_param.receivedKnockbackUpwardForce;
    //ノックバック後の硬直
    m_knockbackTimer = m_param.receivedKnockbackDuration;
}

//----------------------------------------------------------
// AIの思考処理
//----------------------------------------------------------

void BossEnemy::UpdateAI(
    float dt,
    const DirectX::SimpleMath::Vector3& playerPos,
    Stage* stage,
    Particle* /*particle*/,
    EnemyManager* enemyManager)
{
	//攻撃のクールダウンタイマーを減らす
    if (m_meleeTimer > 0.0f) m_meleeTimer -= dt;
    if (m_rushTimer > 0.0f)  m_rushTimer -= dt;

    //毎フレームの最初は歩かない状態にリセット
    m_targetVelocity = DirectX::SimpleMath::Vector3::Zero;

    //Loading中はAIを動かさない
    if (m_state == EnemyState::Opening || m_state == EnemyState::Loading)
    {
        return;
    }

    //プレイヤーへの方向ベクトルを計算
    float dist = DirectX::SimpleMath::Vector3::Distance(m_position, playerPos);
	DirectX::SimpleMath::Vector3 forward = playerPos - m_position;
    forward.y = 0.0f;

    //プレイヤーの方を向く
    if (forward.LengthSquared() > VECTOR_EPSILON)
    {
		//向きベクトルを正規化して角度を計算
        forward.Normalize();
    }

    //ノックバック中か
    if (m_knockbackTimer > 0.0f)
    {
        //敵の行動を止める 
        return;
    }

    //現在の状態ごとの行動パターン
    switch (m_state)
    {
    case EnemyState::Idle:
    {
        //プレイヤーとの距離が一定以上離れていたら近づく
        if (dist > STOP_DISTANCE)
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

		//攻撃のクールダウンが終わっていて、プレイヤーが近ければ攻撃する
        if (enemyManager && !m_attackPatterns.empty())
        {
			//近距離攻撃と突進攻撃の切り替え距離を設定
            if (dist < ATTACK_SWITCH_DISTANCE && m_meleeTimer <= 0.0f)
            {
				m_attackPatterns[0]->Execute(this, forward, enemyManager);
                //敵の攻撃状態
                SetState(EnemyState::Attack, ATTACK_DURATION);
                m_meleeTimer = ATTACK_TIMER;
            }
			//プレイヤーと少し遠ければ突進攻撃
            else if (dist < RUSH_SWITCH_DISTANCE && m_rushTimer <= 0.0f)
            {
                m_attackPatterns[1]->Execute(this, forward, enemyManager);
                //敵の攻撃状態
				SetState(EnemyState::Rush, ATTACK_DURATION);
                m_rushTimer = RUSH_TIMER;
            }
        }
        break;
    }
    //攻撃中はAIは何もしない
    case EnemyState::Rush:
    case EnemyState::Attack:
    default:
        break;

	//回避状態の行動パターン
    case EnemyState::Avoid:
    {
        //プレイヤーから見て右方向を計算
        DirectX::SimpleMath::Vector3 rightDir =
            DirectX::SimpleMath::Vector3::Up.Cross(forward);
        //右方向に歩く
        m_targetVelocity = rightDir;
		break;
    }
    }
}

//----------------------------------------------------------
// 敵の物理演算と移動
//----------------------------------------------------------

void BossEnemy::UpdatePhysics(Stage* stage, float dt, Particle* particle)
{
    CharacterBase::UpdatePhysice(dt, stage);

    if(stage)
    {
        CheckAndHandleFalling(stage, particle);
	}

    //------------------------------------------------
    //ステージの傾きとAIの歩行速度の結合
    //------------------------------------------------
    
    if (stage)
    {
        //ステージから現在の傾きを取得
        DirectX::SimpleMath::Vector3 slideDir = 
            stage->GetSlideDirection(m_position.x,m_position.z);
        //自発的な移動と波の傾きを渡し、座標を更新
        m_slideBehavior.Update(m_position, m_targetVelocity, slideDir, dt);
        //地面との当たり判定
        //敵のいる座標の足場の高さを取得
        float groundY = stage->GetGroundHeight(m_position.x, m_position.z);
        //足場より下、かつ足場の底より上なら着地
        if (m_position.y<groundY && groundY>STAGE_BOUNDARY_Y)
        {
            //ノックバックをリセット
            m_knockbackVelocity.y = 0.0f;

            //最初の演出中なら着地した瞬間にLoadingへ
            if (m_state == EnemyState::Opening)
            {
                SetState(EnemyState::Loading, SMOKE_DURATION);
            }
        }
		//落下の判定と処理
		CheckAndHandleFalling(stage, nullptr);
    }
}

//----------------------------------------------------------
// ボスの状態を変更、その状態を維持するタイマーを設定
//----------------------------------------------------------

void BossEnemy::SetState(EnemyState state, float duration)
{
	//状態を変更
    m_state = state;
    m_stateTimer = duration;
}
