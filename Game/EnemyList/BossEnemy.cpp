#include "pch.h"
#include "BossEnemy.h"
#include <Effects.h>

#include <Game/SoundList/AudioManager.h>

#include <Game/Effects/Particle.h>
#include <Game/Effects/Smoke.h>

using namespace DirectX;

//----------------------------------------------------------
// ボスのインスタンスを生成
//----------------------------------------------------------

BossEnemy::BossEnemy(
    DX::DeviceResources* deviceResources,
    const SimpleMath::Vector3& position,
    float hp,
    float speed,
    std::shared_ptr<DisplayCollision> displayCollision
)
    : m_deviceResources(deviceResources)
    , m_position(position)
    , m_hp(hp)
    , m_maxHp(hp)
    , m_speed(speed)
    , m_displayCollision(displayCollision)
    , m_isSmokeActive(false)
    , m_isLandingEffectDone(false)
    , m_smokeTimer(0.0f)
    , m_isGroundPrev(false)
    , m_isGroundNow(false)
{
}

//----------------------------------------------------------
// モデルやエフェクト、サウンドなどの初期化処理
//----------------------------------------------------------

void BossEnemy::Initialize()
{
    auto device = m_deviceResources->GetD3DDevice();

    EffectFactory fx(device);
    fx.SetDirectory(L"Resources\\Models");

    m_model = Model::CreateFromSDKMESH(
        device,
        L"Resources\\Models\\Pen_Stand.sdkmesh",
        fx
    );

    //各モデルのロード
    m_modelIdle      = DirectX::Model::CreateFromSDKMESH(device, L"Resources\\Models\\Pen_Stand.sdkmesh", fx);
    m_modelAttack    = DirectX::Model::CreateFromSDKMESH(device, L"Resources\\Models\\PenAttack.sdkmesh", fx);
    m_modelShoot     = DirectX::Model::CreateFromSDKMESH(device, L"Resources\\Models\\Pen_Shoot.sdkmesh", fx);
    m_modelRush      = DirectX::Model::CreateFromSDKMESH(device, L"Resources\\Models\\Pen_Rush.sdkmesh",  fx);
    //回避時
    m_modelAwakening = DirectX::Model::CreateFromSDKMESH(device, L"Resources\\Models\\Awakening.sdkmesh", fx);

    //初期状態はIdle
    m_currentModel = m_modelIdle.get();
    m_state = EnemyState::Opening;


    if (m_model)
    {
        m_collision = std::make_unique<ModelCollisionOrientedBox>(m_model.get());
    }

    m_states = std::make_unique<CommonStates>(device);


    //音の読み込み
    AudioManager::GetInstance()->LoadSound("AttackE", L"Resources/Sounds/E_近距離攻撃.wav");
    AudioManager::GetInstance()->LoadSound("DashE", L"Resources/Sounds/E_突進攻撃.wav");
    AudioManager::GetInstance()->LoadSound("BulletE", L"Resources/Sounds/P_E_遠距離攻撃.wav");

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
    const SimpleMath::Vector3& playerPosition,
    float playerSpeed,
    Stage* stage,
    Particle* particle)
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
            m_currentModel = m_modelIdle.get();
        }
    }
    
    //------------------------------------------------
    //AIによる思考処理
    //------------------------------------------------
    UpdateAI(playerPosition);

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
        if (m_smokeTimer >= 1.0f)
        {
            m_isSmokeActive = false;
            
        }
    }

    //------------------------------------------------
    //落下時のリスポーン処理
    //------------------------------------------------

    //落下リスポーン処理
    if (m_position.y < FALL_LIMIT_Y)
    {

        // 水しぶき
        if (particle) // 引数で受け取っている前提
        {
            SimpleMath::Vector3 splashPos = m_position;
            splashPos.y = 0.0f;
            
            //水しぶきの発生
            particle->Spawn(Particle::Type::Splash, m_position, static_cast<int>(SPLASH_PARTICLE_COUNT));
            
        }

        //復帰座標を設定
        m_position = SimpleMath::Vector3(0.0f, RESPAWN_HEIGHT, 0.0f);

        //挙動をリセット
        m_velocity = SimpleMath::Vector3::Zero;
        m_knockbackVelocity = SimpleMath::Vector3::Zero;
        m_knockbackTimer = 0.0f;

        TakeDamage(FALL_DAMAGE, PlayerAttackType::Attack);

        //効果音
        AudioManager::GetInstance()->Play("Fall");

    }

    //------------------------------------------------
    //当たり判定の座標更新
    //------------------------------------------------
    // OBB 更新
    if (m_collision)
    {
        SimpleMath::Matrix world =
            SimpleMath::Matrix::CreateScale(MODEL_RENDER_SCALE)
            * SimpleMath::Matrix::CreateTranslation(m_position);

        m_collision->UpdateBoundingInfo(world);
    }

}

//----------------------------------------------------------
// ボスのモデルやエフェクト、デバッグ用コリジョンを描画
//----------------------------------------------------------

void BossEnemy::Render(
    ID3D11DeviceContext* context,
    const SimpleMath::Matrix& view,
    const SimpleMath::Matrix& proj
    )
{
    if (IsDead()) return;
   
    //
    SimpleMath::Matrix rotX = SimpleMath::Matrix::Identity;

    //突進時に傾ける
    if (m_state == EnemyState::Rush)
    {
        rotX = SimpleMath::Matrix::CreateRotationX(DirectX::XMConvertToRadians(-90.0f));
    }


    // Y軸の回転行列 (計算した角度)
    SimpleMath::Matrix rotation = SimpleMath::Matrix::CreateRotationY(m_rotationY+DirectX::XM_PI);

    // ワールド行列の作成: スケール -> 回転 -> 平行移動
    SimpleMath::Matrix world =
        SimpleMath::Matrix::CreateScale(MODEL_RENDER_SCALE) *
        rotX *
        rotation *
        SimpleMath::Matrix::CreateTranslation(m_position);
    
    //モデルの描画
    m_currentModel->Draw(context, *m_states, world, view, proj);

    m_displayCollision->DrawCollision(
        context, m_states.get(), view, proj,
        Colors::White, Colors::Blue, 0.15f
    );

    //回避時　限定
    if (m_consecutiveHitCount >= EVADE_HIT_THRESHOLD)
    {
        m_modelAwakening->Draw(context, *m_states, world, view, proj);
    }

    //煙の描画
    if (m_isSmokeActive)
    {
        // 時間経過に合わせて大きくする 
        float scale = 3.0f + m_smokeTimer * SMOKE_GROWTH_RATE;

        // 時間経過に合わせて透明にする 
        float alpha = 1.0f - (m_smokeTimer / SMOKE_DURATION);

        // 煙の描画実行
        DirectX::SimpleMath::Vector3 drawPos = m_position;
        drawPos.y += SMOKE_Y_OFFSET;

        m_smokeEffect->Render(view, proj, drawPos, scale, alpha);

        
    }


    if (m_collision)
    {
        //敵のコリジョンを描画 
        m_collision->AddDisplayCollision(m_displayCollision.get()); 

        m_displayCollision->DrawCollision(
            context, m_states.get(), view, proj,
            Colors::White, Colors::Blue, 0.5f // 敵は青色で表示
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
        m_consecutiveHitCount = 1;
        //最後に受けた攻撃のタイプ
        m_lastAttackType = type;
    }

    //回避発動 3回連続で同じ攻撃だったら
    if (m_consecutiveHitCount >= 3)
    {
        
        //回避状態へ
        SetState(EnemyState::Avoid, EVADE_DURATION);

        return;
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
    //敵へのノックバックは軽めに設定 
    m_knockbackVelocity = direction * (power * KNOCKBACK_POWER_SCALE);
    //ノックバック持続時間を適用
    m_knockbackTimer = KNOCKBACK_DURATION;

}

//----------------------------------------------------------
// AIの思考処理
//----------------------------------------------------------

void BossEnemy::UpdateAI(const DirectX::SimpleMath::Vector3& playerPos)
{
    //毎フレームの最初は歩かない状態にリセット
    m_targetVelocity = DirectX::SimpleMath::Vector3::Zero;

    //Loading中はAIを動かさない
    if (m_state == EnemyState::Opening || m_state == EnemyState::Loading)
    {
        return;
    }

    //プレイヤーへの方向ベクトルを計算
    DirectX::SimpleMath::Vector3 dirToPlayer = playerPos - m_position;
    dirToPlayer.y = 0.0f;

    //プレイヤーの方を向く
    if (dirToPlayer.LengthSquared() > 0.0001f)
    {
        //atan2でZ軸回転角度を求める
        m_rotationY = std::atan2(dirToPlayer.x, dirToPlayer.z);
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
        if (dirToPlayer.LengthSquared() > (STOP_DISTANCE * STOP_DISTANCE))
        {
            //純粋な向きだけにする
            dirToPlayer.Normalize();

            //歩きたい速度をセット
            m_targetVelocity = dirToPlayer * m_speed;
        }
        break;

    }
    //攻撃中はAIは何もしない
    case EnemyState::Rush:
    case EnemyState::Attack:
    case EnemyState::Shoot:
    default:
        break;
    }


}

//----------------------------------------------------------
// 敵の物理演算と移動
//----------------------------------------------------------

void BossEnemy::UpdatePhysics(Stage* stage, float dt)
{
    //------------------------------------------------
    //ノックバック処理
    //------------------------------------------------
    if (m_knockbackTimer > 0.0f)
    {
        m_knockbackTimer -= dt;

        //ノックバックの強制的な速度を座標に足す
        m_position += m_knockbackVelocity * dt;

        //摩擦を掛けてノックバックの威力を落とす
        m_knockbackVelocity -= m_knockbackVelocity * KNOCKBACK_DRAG * dt;

        //勢いがほとんどなくなったら停止
        if (m_knockbackVelocity.LengthSquared() < 0.01f)
        {
            m_knockbackVelocity = DirectX::SimpleMath::Vector3::Zero;
        }
    }

    //------------------------------------------------
    //重力の適用
    //------------------------------------------------
    
    //落下速度を重力加速度を足す
    m_velocity.y += m_gravity * dt;
    //Y座標を更新
    m_position.y += m_velocity.y * dt;

    //------------------------------------------------
    //ステージの傾きとAIの歩行速度の結合
    //------------------------------------------------
    
    if (stage)
    {
        //ステージから現在の傾きを取得
        DirectX::SimpleMath::Vector3 slideDir = stage->GetSlideDirection();

        //自発的な移動と波の傾きを渡し、座標を更新
        m_slideBehavior.Update(m_position, m_targetVelocity, slideDir, dt);

        //地面との当たり判定
        //敵のいる座標の足場の高さを取得
        float groundY = stage->GetGroundHeight(m_position.x, m_position.z);

        //足場より下、かつ足場の底より上なら着地
        if (m_position.y<groundY && groundY>STAGE_BOUNDARY_Y)
        {
            //地面の上を押し上げる
            m_position.y = groundY - GROUND_HEIGHT_OFFSET;

            //落下速度をリセット
            m_velocity.y = 0.0f;

            //ノックバックをリセット
            m_knockbackVelocity.y = 0.0f;

            //最初の演出中なら着地した瞬間にLoadingへ
            if (m_state == EnemyState::Opening)
            {
                m_state = EnemyState::Loading;
                m_currentModel = m_modelIdle.get();
            }
        }
    }
    //ステージ情報が取れなかった場合の簡易的な着地判定
    else if (m_position.y <= STAGE_BOUNDARY_Y)
    {
        m_position.y = STAGE_BOUNDARY_Y + GROUND_HEIGHT_OFFSET;

        m_velocity.y = 0.0f;
    }
}

//----------------------------------------------------------
// ボスの状態を変更、その状態を維持するタイマーを設定
//----------------------------------------------------------

void BossEnemy::SetState(EnemyState state, float duration)
{
    m_state = state;
    m_stateTimer = duration;

    switch (m_state)
    {
        //上から落ちてきている
    case EnemyState::Opening:
        m_currentModel = m_modelIdle.get();
        break;
        //着地後止まる
    case EnemyState::Loading:
        m_currentModel = m_modelIdle.get();
        break;
        //通常
    case EnemyState::Idle:  
        m_currentModel = m_modelIdle.get(); 
        break;
        //叩く
    case EnemyState::Attack:
        m_currentModel = m_modelAttack.get();
        break;
        //発射
    case EnemyState::Shoot:
        m_currentModel = m_modelShoot.get();
        break;
        //突進
    case EnemyState::Rush: 
        m_currentModel = m_modelRush.get(); 
        break;
    }
}
