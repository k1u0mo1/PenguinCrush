#include "pch.h"

#include "Player.h"

#include "GimmickList/Stage.h"
#include <Effects/Particle.h>

//影用
#include "DDSTextureLoader.h"

//効果音
#include "SoundList/AudioManager.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

//-----------------------------------------------------------------
//コンストラクタ
//-----------------------------------------------------------------
Player::Player(
    DX::DeviceResources* deviceResources,
    std::shared_ptr<DisplayCollision> collision,
    PlayerCamera* camera)
    : m_deviceResources(deviceResources)
    , m_displayCollision(collision)
    , m_camera(camera)
{
}

//-----------------------------------------------------------------
//初期化関連
//-----------------------------------------------------------------
void Player::Initalize(
    HWND /*hwnd*/, int width, int height, Stage* stage)
{
    CreateDeviceResources();
    CreateWindowSizeResources(width, height);

    //ステージ情報のセット
    m_stage = stage;

    //ステートとモデルのリセット
    m_state = PlayerState::Idle;
    m_currentModel = m_modelIdle.get(); 
    m_stateTimer = 0.0f;

    //初期位置や向きの設定
    m_position = { -10.0f, 0.3f, -10.0f };
    m_forward = { 0.0f, 0.0f, 1.0f };

    //パラメータのリセット
    //重力
    m_velocity = Vector3::Zero;
    //滑る
    m_slidingInertia = Vector3::Zero;
    //ノックバック
    m_knockbackVelocity = Vector3::Zero;
    m_knockbackTimer = 0.0f;
    //ダッシュしているか
    m_isDashing = false;
    //クールタイマー
    m_attackCoolTime = 0.0f;
    
    //ステータスのリセット
    //体力
    m_stats.hp = m_stats.hp_Max;
    //スタミナ
    m_stats.stamina = m_stats.stamina_Max;
    //弾数
    m_stats.ammo = m_stats.ammo_Max;
}

//-----------------------------------------------------------------
//更新関連
//-----------------------------------------------------------------
void Player::Update(
    float elapsedTime, 
    const Mouse::State& mouse,
    const Mouse::ButtonStateTracker& mouseTracker, 
    Stage* stage,
    Wave* wave,
    Particle* particle)
{
    if (!stage) return;

    // 状態タイマーの更新
    if (m_stateTimer > 0.0f)
    {
        m_stateTimer -= elapsedTime;

        if (m_stateTimer <= 0.0f)
        {
            m_state = PlayerState::Idle;
            m_currentModel = m_modelIdle.get();
        }
    }

    //クールタイムを減らす
    if (m_attackCoolTime > 0.0f)
    {
        m_attackCoolTime -= elapsedTime;
    }

    //重力
    m_velocity.y += m_gravity * elapsedTime;
    m_position.y += m_velocity.y * elapsedTime;

    //ノックバック処理
    if (m_knockbackTimer > 0.0f)
    {
        m_knockbackTimer -= elapsedTime;

        //ノックバックによる強制移動
        m_position += m_knockbackVelocity * elapsedTime;

        //速度を減衰させる 
        m_knockbackVelocity -= m_knockbackVelocity * KNOCKBACK_DRAG * elapsedTime;

        //ノックバック中に地面にめり込まないように高さ補正
        float stageY = stage->GetGroundHeight(m_position.x, m_position.z);
        if (m_position.y < stageY)
        {
            m_position.y = stageY;
            m_knockbackVelocity.y = 0.0f;
        }
    }

    //移動
    HandleMovement(elapsedTime, stage,particle);

    //攻撃
    auto kb = DirectX::Keyboard::Get().GetState();
    HandleAttack(elapsedTime, mouse, mouseTracker, kb);

    //スタミナ回復
    UpdateStamina(elapsedTime);

    //落下して波の下に到達した場合のリスポーン処理
    if (wave && m_position.y < -5.0f) // -5.0f は波の平均高さ(-4.0f)より少し下
    {

        //水しぶきエフェクト発生
        if (particle)
        {
            // 水面の高さ（例: -4.0f）に合わせて発生位置を調整しても良い
            SimpleMath::Vector3 splashPos = m_position;
            splashPos.y = 0.0f;
            //水しぶき
            particle->Spawn(Particle::Type::Splash,splashPos, 30); // 30個飛ばす

            AudioManager::GetInstance()->Play("Fall");
        }

        // プレイヤーを安全な位置
        m_position = { 0.0f, 0.0f, 0.0f };
        m_velocity = { 0.0f, 0.0f, 0.0f };
        m_knockbackVelocity = { 0.0f, 0.0f, 0.0f };
        m_knockbackTimer = 0.0f;

        //HPを減らすなどのペナルティ処理
        m_stats.TakeDamage(50.0f); 
    }

    // コリジョン情報の更新を追加
    if (m_collision)
    {
        // プレイヤーの姿勢（回転と位置）を含むワールド行列を構築
        float angleY = atan2(m_forward.x, m_forward.z);
        SimpleMath::Matrix world =
            SimpleMath::Matrix::CreateRotationY(angleY)
            * SimpleMath::Matrix::CreateTranslation(m_position);

        m_collision->UpdateBoundingInfo(world);
    }
}

//-----------------------------------------------------------------
//描画関連
//-----------------------------------------------------------------
void Player::Render(ID3D11DeviceContext* context,
    DirectX::SimpleMath::Matrix view, 
    DirectX::SimpleMath::Matrix proj,
    ShadowRenderer* shadowRenderer)
{
    auto kb = DirectX::Keyboard::Get().GetState();

    if (!m_model)
    {
        return;
    }
    
    //カメラ方向にプレイヤーを向かせる
    float angleY = atan2(m_forward.x, m_forward.z);

    DirectX::SimpleMath::Matrix rot = DirectX::SimpleMath::Matrix::CreateRotationY(angleY + DirectX::XM_PI);

    //モデルの角度を変更する用
    SimpleMath::Matrix rotX = SimpleMath::Matrix::Identity;

    if (kb.Space && kb.W && m_stats.stamina >= 0.0f)
    {
        // 即時90度
        rotX = SimpleMath::Matrix::CreateRotationX(XMConvertToRadians(-90.0f));

    }
    else
    {
        rotX = SimpleMath::Matrix::Identity;
    }

    DirectX::SimpleMath::Matrix world =
        rotX*rot * DirectX::SimpleMath::Matrix::CreateTranslation(m_position);

    //---------------------------------------------------
    //影の描画  途中
    //---------------------------------------------------
    if (m_stage && shadowRenderer)
    {
        //ステージの情報を取得 座標
        SimpleMath::Vector3 shadowPos = m_position;

        //影の大きさ
        float shadowScale = 2.0f;

        //ステージの傾きを取得
        float stagerotX = m_stage->GetRotateX();
        float stagerotZ = m_stage->GetRotateZ();

        //影を描画
        shadowRenderer->Render(
            context,
            m_states.get(),
            view,
            proj,
            shadowPos,
            shadowScale,
            stagerotX,
            stagerotZ
        );
    }

    //モデルの描画
    //m_model->Draw(context, *m_states, world, view, proj);

    m_currentModel->Draw(context, *m_states, world, view, proj);
    
    if (m_displayCollision && m_collision) 
    {
        //DisplayCollision に現在のコリジョン情報を登録 
        m_collision->AddDisplayCollision(m_displayCollision.get());


        //当たり判定の描画-----------------------------------------
        
        //登録されたコリジョンを描画
        m_displayCollision->DrawCollision(
            context, m_states.get(), view, proj,
            Colors::Green, Colors::Lime, 0.15f // プレイヤーは緑色で表示
        );
       
        //---------------------------------------------------------
    }

}

//-----------------------------------------------------------------
//攻撃を受けたときにノックバック
//-----------------------------------------------------------------
void Player::ApplyKnockback(const DirectX::SimpleMath::Vector3& direction, float power)
{
    SimpleMath::Vector3 kbDir = direction;
    //ベクトルを加える（上方向）
    kbDir.y = 0.5f;

    kbDir.Normalize();

    // プレイヤーへのノックバック設定
    m_knockbackVelocity = kbDir * (power * 10.0f);
    // 0.2秒間ノックバックを適用
    m_knockbackTimer = 0.2f; 
    // ノックバック中はダッシュを解除
    m_isDashing = false; 

}

//-----------------------------------------------------------------
//ダッシュの計算
//-----------------------------------------------------------------
float Player::GetMoveSpeed() const
{
    // Player::HandleMovement で定義されている速度定数
    const float moveSpeed = 5.0f;
    const float dashSpeed = 30.0f;

    if (m_isDashing)
    {
        return moveSpeed + dashSpeed; // 5.0f + 30.0f = 35.0f
    }

    return moveSpeed; // 通常速度
}

//-----------------------------------------------------------------
//移動関連
//-----------------------------------------------------------------
void Player::HandleMovement(float elapsedTime, Stage* stage, Particle* particle)
{
    if (!stage) return;

    // ダッシュ判定（スペースキー）
    auto kb = DirectX::Keyboard::Get().GetState();
    
    //通常速度
    const float moveSpeed = 5.0f;
    //ダッシュ速度
    const float dashSpeed = 30.0f;

    //ノックバック中は移動入力を無視
    if (m_knockbackTimer > 0.0f)
    {
        m_isDashing = false;
        return;
    }


    //カメラ方向の取得
    DirectX::SimpleMath::Vector3 camForward = m_camera->GetTargetPosition() - m_camera->GetEyePosition();
    camForward.y = 0;
    camForward.Normalize();

    //プレイヤーの forward をカメラ方向に更新
    float turnSpeed = 5.0f; // 追従スピード
    m_forward = m_forward + (camForward - m_forward) * turnSpeed * elapsedTime;
    m_forward.Normalize();

    DirectX::SimpleMath::Vector3 forwardVector = m_forward;
    
    //ダッシュしてるか
    if (kb.Space && kb.W && m_stats.stamina > 0.0f)
    {
        m_isDashing = true;
    }
    else
    {
        m_isDashing = false;
    }

    //----------------------------------------------------
    //移動
    //----------------------------------------------------
    
    // 入力に基づく「出したい速度」を計算
    Vector3 targetVelocity = Vector3::Zero;

    if (kb.W/* ||kb.A || kb.D*/)
    {
        //走っているか？
        float currentSpeed = m_isDashing ? dashSpeed : moveSpeed;
        //移動計算 あとで合わせる
        targetVelocity = forwardVector * currentSpeed;

        if (m_isDashing )
        {
            //消費するスタミナ量
            m_stats.UseStamina(0.5f);

            // ダッシュ中のエフェクト発生
            if (particle)
            {
                // 描画される高さ
                SimpleMath::Vector3 dashPos = m_position;
                
                //描画
                particle->Spawn(Particle::Type::Dash, dashPos, 10, 0.2);
            }
        }
    }
    else if (kb.S)
    {
        // 後退
        targetVelocity = -forwardVector * moveSpeed * 0.75f;
    }

    ////重力
    //m_velocity.y += m_gravity * elapsedTime;
    //m_position.y += m_velocity.y * elapsedTime;

    //----------------------------------------------------
    //滑る慣性を選ぶ
    //----------------------------------------------------
    //最初の動き始め（加速）
    float acceleration = 10.0f;
    //滑る慣性（減速）
    float friction = 1.0f;

    //入力があるときは加速に、加速がないときは
    float lerpFactor = 
        (targetVelocity.LengthSquared() > 0.01f) ? acceleration : friction;

    //「ダッシュ」 ⇔ 「止まる」時の変化を緩やかにする
    m_slidingInertia = DirectX::SimpleMath::Vector3::Lerp(
        m_slidingInertia,
        targetVelocity,
        lerpFactor * elapsedTime
    );

    //完全に停止させる
    if (m_slidingInertia.LengthSquared() < 0.01f)
    {
        m_slidingInertia = DirectX::SimpleMath::Vector3::Zero;
    }

    //----------------------------------------------------
    //滑る慣性をm_positionに合わせる
    //----------------------------------------------------
    //慣性を座標に合わせる
    m_position += m_slidingInertia * elapsedTime;


    // ステージの高さで補正
    float stageY = stage->GetGroundHeight(m_position.x, m_position.z);

    // 波のリスポーン判定ライン（-5.0f）と合わせて、ステージ上の有効な地面のY座標の最低値を定義
    // この値より stageY が低い場合、ステージ外（奈落）と見なして高さ補正を行わない。
    const float STAGE_BOUNDARY_Y = -4.9f;

    // プレイヤーがステージの地面より下におり、かつ、その地面がステージ上の有効な高さ（STAGE_BOUNDARY_Yより高い）である場合のみ補正を行う
    if (m_position.y < stageY && stageY > STAGE_BOUNDARY_Y)
    {
        float heightOffset = 0.5f;

        m_position.y = stageY-heightOffset;
        m_velocity.y = 0.0f;
    }

    // スライド（傾斜）を適用
    Vector3 slideDir = stage->GetSlideDirection();
    m_position += slideDir * elapsedTime;
    
}

//-----------------------------------------------------------------
//プレイヤーの攻撃関連
//-----------------------------------------------------------------
void Player::HandleAttack(
    float elapsedTime,
    const DirectX::Mouse::State& /*mouse*/,
    const DirectX::Mouse::ButtonStateTracker& mouseTracker,
    const DirectX::Keyboard::State& kb)
{
    //------------------------------------------------------
    //攻撃系
    //------------------------------------------------------
    
    // 近距離攻撃（左クリック）
    if (mouseTracker.leftButton == Mouse::ButtonStateTracker::PRESSED &&
        m_stats.stamina >= 10.0f)
    {
        //モデル
        m_state = PlayerState::Attack;
        m_currentModel = m_modelAttack.get();
        m_stateTimer = 0.3f;

        if (m_attackManager)
            m_attackManager->Attack(this);

        m_stats.UseStamina(15.0f);
    }

    // 遠距離攻撃（右クリック）
    if (mouseTracker.rightButton == Mouse::ButtonStateTracker::PRESSED &&
        m_stats.stamina >= 5.0f &&
        m_stats.ammo > 0)
    {
        //モデル
        m_state = PlayerState::Shoot;
        m_currentModel = m_modelShoot.get();
        m_stateTimer = 0.2f;

        if (m_attackManager)
            m_attackManager->Bullet(this);

        m_stats.UseStamina(10.0f);

        m_stats.UseAmmo();
    }

    // ラッシュ攻撃（ダッシュ中）
    if (m_state == PlayerState::Rush)
    {
        // スペースを離した、またはスタミナ切れで終了
        if (!kb.Space || m_stats.stamina <= 0.0f)
        {
            m_state = PlayerState::Idle;
            m_currentModel = m_modelIdle.get();
            m_attackCoolTime = 0.5f;
            return;
        }
        // Rush継続中の処理
        m_stats.UseStamina(0.05f * elapsedTime);
        return;
    }
    
    //ダッシュ
    if (m_isDashing && kb.Space && m_stats.stamina >= 30.0f&&m_attackCoolTime<=0.0f)
    {
        m_state = PlayerState::Rush;
        m_currentModel = m_modelRush.get();
        m_stateTimer = 0.0f;

        if (m_attackManager)
            m_attackManager->Rush(this);
        {
            m_stats.UseStamina(0.05f);
            //攻撃のクールタイマー
           // m_attackCoolTime = 0.5f;
        }
    }
}

//-----------------------------------------------------------------
//スタミナの回復
//-----------------------------------------------------------------
void Player::UpdateStamina(float elapsedTime)
{
    // スタミナ自然回復
    m_stats.RecoverStamina(elapsedTime);
}

//-----------------------------------------------------------------
//攻撃を受けたときにHPを減らす
//-----------------------------------------------------------------
void Player::TakeDamage(float amount)
{
    //HPを減らす
    m_stats.hp -= amount;

    //０未満にならないようにする
    if (m_stats.hp < 0.0f)
    {
        m_stats.hp = 0.0f;
    }
}

void Player::CreateDeviceResources()
{
    auto device = m_deviceResources->GetD3DDevice();

    //------------------------------------------------------
    //ステートの作成
    //------------------------------------------------------

    m_states = std::make_unique<CommonStates>(device);

    //------------------------------------------------------
    //モデルの読み込み
    //------------------------------------------------------
    DirectX::EffectFactory fx(device);
    fx.SetDirectory(L"Resources\\Models");

    //各状態のモデルをロード
    m_model = DirectX::Model::CreateFromSDKMESH(device, L"Resources\\Models\\Pen_Stand.sdkmesh", fx);
    m_modelIdle = Model::CreateFromSDKMESH(device, L"Resources\\Models\\Pen_Stand.sdkmesh", fx);
    m_modelAttack = Model::CreateFromSDKMESH(device, L"Resources\\Models\\PenAttack.sdkmesh", fx);
    m_modelShoot = Model::CreateFromSDKMESH(device, L"Resources\\Models\\Pen_Shoot.sdkmesh", fx);
    m_modelRush = Model::CreateFromSDKMESH(device, L"Resources\\Models\\Pen_Rush.sdkmesh", fx);

    m_model = m_modelIdle;

    //------------------------------------------------------
    //コリジョンの生成
    //------------------------------------------------------
    if (m_model)
    {
        //当たり判定
        m_collision = std::make_unique<ModelCollisionOrientedBox>(m_model.get());
    }

    //------------------------------------------------------
    //影用のテクスチャの読み込み
    //------------------------------------------------------

    CreateDDSTextureFromFile(device, L"Resources\\Textures\\Shadow.dds", nullptr, m_shadowTexture.GetAddressOf());

    //------------------------------------------------------
    //効果音のロード
    //------------------------------------------------------
    //AudioManager* audio = AudioManager::GetInstance();

    //攻撃関連
    AudioManager::GetInstance()->LoadSound("Attack", L"Resources/Sounds/平手打ち1.wav");
    AudioManager::GetInstance()->LoadSound("Dash", L"Resources/Sounds/打撃1.wav");
    AudioManager::GetInstance()->LoadSound("Bullet", L"Resources/Sounds/銃を撃つ（パーン）.wav");

    //その他
    AudioManager::GetInstance()->LoadSound("Fall", L"Resources/Sounds/水・ざぶーん04.wav");
    AudioManager::GetInstance()->LoadSound("Reload", L"Resources/Sounds/弾丸を装填.wav");

}

void Player::CreateWindowSizeResources(int /*width*/, int /*height*/)
{
}