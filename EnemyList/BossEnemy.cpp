#include "pch.h"
#include "BossEnemy.h"
#include <Effects.h>

#include <SoundList/AudioManager.h>

#include <Effects/Particle.h>
#include <Effects/Smoke.h>

using namespace DirectX;

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
    AudioManager::GetInstance()->LoadSound("AttackE", L"Resources/Sounds/軽いパンチ1.wav");
    AudioManager::GetInstance()->LoadSound("DashE", L"Resources/Sounds/重いパンチ2.wav");
    AudioManager::GetInstance()->LoadSound("BulletE", L"Resources/Sounds/銃を撃つ（パーン）.wav");

    AudioManager::GetInstance()->LoadSound("Fall", L"Resources/Sounds/水・ざぶーん04.wav");

    //エフェクト系
    //煙の生成と初期化
    m_smokeEffect = std::make_unique<Smoke>();
    m_smokeEffect->Initialize(m_deviceResources);

    m_isSmokeActive = false;
    m_smokeTimer = 0.0f;

    //着地している
    m_isGroundPrev = true;

    
}

void BossEnemy::Update(float deltaTime,
    const SimpleMath::Vector3& playerPosition,
    float playerSpeed,
    Stage* stage,
    Particle* particle)
{
    if (IsDead()) return;

    // タイマーが動いていれば減らす
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
    
   
    // 現在の移動ベクトルを格納する変数
    SimpleMath::Vector3 totalMovement = SimpleMath::Vector3::Zero;


    //敵がOpening以外の時に適応される
    if (m_state != EnemyState::Opening )
    {

        //プレイヤーへの方向ベクトルを計算 
        DirectX::SimpleMath::Vector3 direction = playerPosition - m_position;
        direction.y = 0.0f;

        if (direction.LengthSquared() > 0.0001f)
        {

            // atan2(X, Z) は、Z軸(0度)を基準に時計回りに回転する角度 (ラジアン) を返します
            float yaw = std::atan2(direction.x, direction.z);

            // m_rotationY に保存
            m_rotationY = yaw;
        }

    }

    if (m_state != EnemyState::Opening && m_state != EnemyState::Loading)
    {

        // ノックバック処理
        if (m_knockbackTimer > 0.0f)
        {
            m_knockbackTimer -= deltaTime;

            // ノックバックによる強制移動を加算
            totalMovement += m_knockbackVelocity * deltaTime;

            // 速度を減衰させる (Playerと同じロジックを適用)
            m_knockbackVelocity -= m_knockbackVelocity * KNOCKBACK_DRAG * deltaTime;

            if (m_knockbackTimer <= 0.0f)
            {
                m_knockbackVelocity = SimpleMath::Vector3::Zero;
            }
        }

        // AI追尾処理 (ノックバック中でない時のみ)
        if (m_knockbackTimer <= 0.0f) // ノックバックがゼロ以下の場合のみ追尾
        {
            //プレイヤーへの方向ベクトルを計算
            SimpleMath::Vector3 moveDir = playerPosition - m_position;
            moveDir.y = 0.0f;

            // プレイヤーとの距離チェック
            if (moveDir.LengthSquared() > 2.0f)
            {
                moveDir.Normalize();

                float currentSpeed = m_speed;
                if (playerSpeed > m_speed)
                {
                    currentSpeed = playerSpeed * 0.5f;
                }

                // AI追尾による移動を加算
                totalMovement += moveDir * currentSpeed * deltaTime;
            }
        }

    }

    // 全ての水平移動を位置に適用
    m_position += totalMovement;

    // 重力とステージ高さ補正

    // 重力
    m_velocity.y += m_gravity * deltaTime;
    m_position.y += m_velocity.y * deltaTime; // Y軸はここで更新

    //スライド
    DirectX::SimpleMath::Vector3 slideDir = stage->GetSlideDirection();
    m_position += slideDir * deltaTime; // スライド移動を位置に反映

    // ステージの高さに合わせる
    float groundY = stage->GetGroundHeight(m_position.x, m_position.z);
    //境界
    const float STAGE_BOUNDARY_Y = -4.9f;

    if (m_position.y < groundY && groundY > STAGE_BOUNDARY_Y)
    {
       
        float heightOffset = 0.5f;

        m_position.y = groundY - heightOffset;
       //落下速度リセット
       m_velocity.y = 0.0f;
       // ノックバック中に地面に衝突したらY方向のノックバックを停止
       m_knockbackVelocity.y = 0.0f; 

       //着地したら敵のOpeningー＞Loadingへ変更する
       if (m_state == EnemyState::Opening)
       {
           //ステートを変更
           m_state = EnemyState::Loading;
           m_currentModel = m_modelIdle.get();

          
       }
       
    }

    //着地判定
    m_isGroundNow = (m_position.y <= 7.0f);

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

    //落下リスポーン処理
    const float FALL_LIMIT = -5.0f;
    if (m_position.y < FALL_LIMIT)
    {

        // 水しぶき
        if (particle) // 引数で受け取っている前提
        {
            SimpleMath::Vector3 splashPos = m_position;
            splashPos.y = 0.0f;
            
            particle->Spawn(Particle::Type::Splash, m_position, 50);
            
        }

        //復帰座標を設定
        m_position = SimpleMath::Vector3(0.0f, 10.0f, 0.0f);

        //挙動をリセット
        m_velocity = SimpleMath::Vector3::Zero;
        m_knockbackVelocity = SimpleMath::Vector3::Zero;
        m_knockbackTimer = 0.0f;

        TakeDamage(400.0f,PlayerAttackType::Attack);

        //効果音
        AudioManager::GetInstance()->Play("Fall");

    }

    

    // OBB 更新
    if (m_collision)
    {
        SimpleMath::Matrix world =
            SimpleMath::Matrix::CreateScale(1.7f)
            * SimpleMath::Matrix::CreateTranslation(m_position);

        m_collision->UpdateBoundingInfo(world);
    }

}

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
        SimpleMath::Matrix::CreateScale(1.7f) *
        rotX *
        rotation *
        SimpleMath::Matrix::CreateTranslation(m_position);

    ////--------------------------------------------
    ////影の描画
    ////--------------------------------------------
    //if (shadowRenderer)
    //{
    //    //影の位置
    //    DirectX::SimpleMath::Vector3 shadowPos = m_position;
    //    //高さをY軸に合わせる
    //    shadowPos.y = 0.0f;
    //    //影の大きさ
    //    float shadowScale = 2.5f;
    //    // 丸影の描画実行
    //    shadowRenderer->Render(
    //        context,
    //        m_states.get(),
    //        view,
    //        proj,
    //        shadowPos,   
    //        shadowScale 
    //    );
    //}

    //m_model->Draw(context, *m_states, world, view, proj);

    m_currentModel->Draw(context, *m_states, world, view, proj);

    m_displayCollision->DrawCollision(
        context, m_states.get(), view, proj,
        Colors::White, Colors::Blue, 0.15f
    );

    //回避時　限定
    if (m_consecutiveHitCount >= 3)
    {
        m_modelAwakening->Draw(context, *m_states, world, view, proj);
    }

    //煙の描画
    if (m_isSmokeActive)
    {
        // 時間経過に合わせて大きくする 
        float scale = 3.0f + m_smokeTimer * 100.0f;

        // 時間経過に合わせて透明にする 
        float alpha = 1.0f - (m_smokeTimer / 1.0f); 

        // 煙の描画実行
        DirectX::SimpleMath::Vector3 drawPos = m_position;
        drawPos.y += 0.5f; 

        m_smokeEffect->Render(view, proj, drawPos, scale, alpha);

        
    }


    if (m_collision)
    {
        //敵のコリジョンを描画 
        m_collision->AddDisplayCollision(m_displayCollision.get()); // この行を追加する!

        m_displayCollision->DrawCollision(
            context, m_states.get(), view, proj,
            Colors::White, Colors::Blue, 0.5f // 敵は青色で表示
        );
    }
}

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
        //カウントをリセット
        //m_consecutiveHitCount = 0;

        //回避状態へ
        SetState(EnemyState::Avoid, 0.5f);

        return;
    }

    //HPを減らす
    m_hp -= amount;

    if (m_hp < 0) m_hp = 0;
    
}

//攻撃を受けたときにノックバック
void BossEnemy::ApplyKnockback(const DirectX::SimpleMath::Vector3& direction, float power)
{
    // 敵へのノックバックは軽めに設定 
    m_knockbackVelocity = direction * (power * 10.0f);
    // 0.15秒間ノックバックを適用
    m_knockbackTimer = 0.15f; 

}

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
