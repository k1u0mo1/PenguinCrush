
/**
 * @file   Player.cpp
 * @brief  プレイヤーキャラクターの制御・パラメータ管理を行うクラス
 * @author 國田知睦
 * @date   2026/06/04
 */

#include "pch.h"
#include "Player.h"

#include "PlayerRenderer.h"
#include "Game/GimmickList/Stage.h"
#include "Game/Effects/Particle.h"
#include "Game/SoundList/AudioManager.h"
#include "Game/Camera/PlayerCamera.h"
#include "Game/ShadowRenderer/ShadowRenderer.h"
#include "Game/GimmickList/WaveManager.h"
#include "AttackList/AttackManager.h"

#include "Game/Collision/ModelCollision.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

//-----------------------------------------------------------------
// プレイヤーインスタンスを生成
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

Player::~Player()
{
}

//-----------------------------------------------------------------
// プレイヤーの初期化
//-----------------------------------------------------------------

void Player::Initialize(
    HWND /*hwnd*/, int width, int height, Stage* stage)
{
    CreateDeviceResources();
    CreateWindowSizeResources(width, height);

    //ステージ情報のセット
    m_stage = stage;

	//描画用のレンダラーの生成と初期化
    m_renderer = std::make_unique<PlayerRenderer>();
    m_renderer->Initialize(m_deviceResources->GetD3DDevice());

    // 当たり判定の生成
    if (m_renderer->GetMainModel()) {
        m_collision = std::make_unique<ModelCollisionOrientedBox>(m_renderer->GetMainModel());
    }

    //ステートとモデルのリセット
    m_state = PlayerState::Idle;
    
	//状態タイマーのリセット
    m_stateTimer = 0.0f;

    //初期位置や向きの設定
    m_position = { 
        PLAYER_START_POSITION_X,
        PLAYER_START_POSITION_Y, 
        PLAYER_START_POSITION_Z };

	//初期の向きはZ軸正方向
    m_forward = { 0.0f, 0.0f, 0.0f };

    //パラメータのリセット
    //重力
    m_velocity = Vector3::Zero;

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
}

//-----------------------------------------------------------------
// プレイヤーの毎フレームの更新処理
//-----------------------------------------------------------------

void Player::Update(
    float elapsedTime,
    const Mouse::State& mouse,
    const Mouse::ButtonStateTracker& mouseTracker,
    Stage* stage,
    WaveManager* /*waveManager*/,
    Particle* particle)
{
    if (!stage) return;

    CheckAndHandleFalling(stage, particle);

    auto kb = DirectX::Keyboard::Get().GetState();

    // 状態タイマーの更新
    if (m_stateTimer > 0.0f)
    {
        m_stateTimer -= elapsedTime;

        if (m_stateTimer <= 0.0f)
        {
            m_state = PlayerState::Idle;
        }
    }

    //ふらつき中ならエフェクトを回転させる
    if (m_state == PlayerState::Dizzy)
    {
        m_dizzyRotationY += DIZZY_EFFECT_ROT_SPEED * elapsedTime;
    }

    //クールタイムを減らす
    if (m_attackCoolTime > 0.0f)
    {
        m_attackCoolTime -= elapsedTime;
    }


    //移動
    HandleMovement(elapsedTime, stage, particle);

    //ノックバック処理
    CharacterBase::UpdatePhysice(elapsedTime, m_stage);

    //攻撃
    HandleAttack(elapsedTime, mouse, mouseTracker, kb);

    //スタミナ回復
    UpdateStamina(elapsedTime);

    
	//落下してステージの下に到達した場合の処理
    if(m_position.y< STAGE_BOUNDARY_Y)
    {
        //効果音
        AudioManager::GetInstance()->Play("Fall");
        if (particle)
        {
            // 水面の高さに合わせて発生位置
            SimpleMath::Vector3 splashPos = m_position;
            splashPos.y = 0.0f;
            //水しぶき
            particle->Spawn(Particle::Type::Splash, splashPos, SPLASH_OF_WATER);
        }

		//安全なリスポーン位置の検索
        DirectX::SimpleMath::Vector3 safePosition;
        if (m_stage && m_stage->FindSafeRespawnPosition(RESPAWN_HEIGHT, STAGE_BOUNDARY_Y, safePosition))
        {
			//安全な位置が見つかった場合はそこにリスポーン
            m_position = safePosition;
        }
        else
        {
			//安全な位置が見つからない場合は、リスポーン高さに固定してリスポーン
            m_position = DirectX::SimpleMath::Vector3(0.0f, RESPAWN_HEIGHT, 0.0f);
        }

		//落下したときの速度をリセット
        m_velocity = DirectX::SimpleMath::Vector3::Zero;
		//ノックバックもリセット
        m_knockbackVelocity = DirectX::SimpleMath::Vector3::Zero;

		//HPを減らすなどのペナルティ処理
        m_stats.TakeDamage(FALL_DAMAGE);

	}


    // コリジョン情報の更新を追加
    if (m_collision)
    {
		//ワールド行列を計算してコリジョンに渡す
        SimpleMath::Matrix world = SimpleMath::Matrix::CreateWorld(m_position, m_forward, SimpleMath::Vector3::Up);

        m_collision->UpdateBoundingInfo(world);
    }
}

//-----------------------------------------------------------------
// プレイヤーモデル、影、当たり判定の描画
//-----------------------------------------------------------------

void Player::Render(ID3D11DeviceContext* context,
    DirectX::SimpleMath::Matrix& view,
    DirectX::SimpleMath::Matrix& proj,
    ShadowRenderer* shadowRenderer)
{
    if (!m_renderer) return;

	//プレイヤーの描画 モデル関連
    m_renderer->Render(
        context,
        m_position,
        m_forward,
        m_state,
        m_dizzyRotationY,
        m_rotationMatrix,
        view,
        proj,
        m_stage,
        shadowRenderer
    );

    
    //当たり判定
    if (m_displayCollision && m_collision)
    {
        //DisplayCollision に現在のコリジョン情報を登録 
        m_collision->AddDisplayCollision(m_displayCollision.get());


        //当たり判定の描画-----------------------------------------

        //登録されたコリジョンを描画
        m_displayCollision->DrawCollision(
            context, m_states.get(), view, proj,
            Colors::Green, Colors::Lime, COLLISION_LINE_THICKNESS // プレイヤーは緑色で表示
        );

        //---------------------------------------------------------
    }

}

//-----------------------------------------------------------------
// プレイヤーに外部からノックバック  敵ー＞プレイヤー
//-----------------------------------------------------------------

void Player::ApplyKnockback(const DirectX::SimpleMath::Vector3& direction, float power)
{
    //ノックバック
    CharacterBase::ApplyKnockback(
        direction, power * KNOCKBACK_POWER_SCALE,
        KNOCKBACK_UP_FORCE, 
        KNOCKBACK_DURATION);

    // ノックバック中はダッシュを解除
    m_isDashing = false;

    //突進中に敵に当たったか？
    if (m_state == PlayerState::Rush)
    {
        //ふらつき状態を呼び出す
        ApplyDizzy();
    }

}

//-----------------------------------------------------------------
// 現在のプレイヤーの移動速度を計算して取得
//-----------------------------------------------------------------

float Player::GetMoveSpeed() const
{

    //ダッシュ中の速度
    if (m_isDashing)
    {
        return MOVE_SPEED + DASH_SPEED; 
    }

    // 通常速度
    return MOVE_SPEED;
}

//-----------------------------------------------------------------
// プレイヤーの移動、ダッシュ、慣性、地形に沿った高さ補正の処理
//-----------------------------------------------------------------

void Player::HandleMovement(float elapsedTime, Stage* stage, Particle* particle)
{
    if (!stage) return;

    // ダッシュ判定（スペースキー）
    auto kb = DirectX::Keyboard::Get().GetState();

    //ノックバック中は移動入力を無視
    if (m_knockbackTimer > 0.0f)
    {
        m_isDashing = false;
        return;
    }

    //ふらつき状態は移動入力を無視
    if (m_state == PlayerState::Dizzy)
    {
        //突進を解除
        m_isDashing = false;

        //ステージの傾斜のみを渡して更新
        Vector3 slideDir = stage->GetSlideDirection(m_position.x,m_position.z);

        m_slideBehavior.Update(m_position, Vector3::Zero, slideDir, elapsedTime);

        float stageY = stage->GetGroundHeight(m_position.x, m_position.z);

        if (m_position.y < stageY && stageY > STAGE_BOUNDARY_Y) {
            m_position.y = stageY - GROUND_OFFSET_Y;
            m_velocity.y = 0.0f;
        }
        return;
    }

    //カメラ方向の取得
    DirectX::SimpleMath::Vector3 camForward = m_camera->GetTargetPosition() - m_camera->GetEyePosition();
    camForward.y = 0;
    camForward.Normalize();

    //プレイヤーの forward をカメラ方向に更新
    m_forward = m_forward + (camForward - m_forward) * TURN_SPEED * elapsedTime;
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

    if (kb.W /*||kb.A || kb.D*/)
    {
        //走っているか？
        float currentSpeed = m_isDashing ? DASH_SPEED : MOVE_SPEED;
        //移動計算 あとで合わせる
        targetVelocity = forwardVector * currentSpeed;

        if (m_isDashing)
        {
            //消費するスタミナ量
            m_stats.UseStamina(STAMINA_COST_DASH * elapsedTime);

            // ダッシュ中のエフェクト発生
            if (particle)
            {
                // 描画される高さ
                SimpleMath::Vector3 dashPos =
                    m_position + SimpleMath::Vector3(0.0f, DASH_HEIGHT,0.0f);
                
                //ダッシュの煙の描画
                particle->Spawn(
                    Particle::Type::Dash,
                    dashPos,
                    DASH_EFFECT_NUM,
                    DASH_EFFECT_SIZE);
            }
        }
    }
    else if (kb.S)
    {
        // 後退
        targetVelocity = -forwardVector * MOVE_SPEED ;
    }


    //----------------------------------------------------
    //滑る慣性を選ぶ
    //----------------------------------------------------

    //ステージからのスライド方向を取得
    Vector3 slideDir = stage->GetSlideDirection(m_position.x,m_position.z);

    //突進（ダッシュ）中かどうかで処理を分ける
    if (m_state == PlayerState::Rush || m_isDashing)
    {
        //突進中は慣性（ステージの滑り）を無視
        m_position += targetVelocity * elapsedTime;

        //ダッシュ中は足場の傾きの慣性を外す
        m_slideBehavior.Update(m_position, Vector3::Zero, slideDir, elapsedTime);
    }
    else
    {
        //通常は足場の傾きを付ける
        m_slideBehavior.Update(m_position, targetVelocity, slideDir, elapsedTime);
    }

    
}

//-----------------------------------------------------------------
//プレイヤーのマウスとキーボード入力に応じた攻撃の処理
//-----------------------------------------------------------------

void Player::HandleAttack(
    float elapsedTime,
    const DirectX::Mouse::State& /*mouse*/,
    const DirectX::Mouse::ButtonStateTracker& mouseTracker,
    const DirectX::Keyboard::State& kb)
{
    //ふらつき中は攻撃できない
    if (m_state == PlayerState::Dizzy) return;

    //------------------------------------------------------
    //攻撃系
    //------------------------------------------------------

    // 近距離攻撃（左クリック）
    if (mouseTracker.leftButton == Mouse::ButtonStateTracker::PRESSED &&
        m_stats.stamina >= STAMINA_REQ_ATTACK &&
        m_state != PlayerState::Rush)
    {
        //モデル
        m_state = PlayerState::Attack;
        
		//攻撃のクールタイマー
        m_stateTimer = STATE_TIMER_ATTACK;

		//攻撃処理
        if (m_attackManager)
            m_attackManager->Attack(this);

		//スタミナを消費
        m_stats.UseStamina(STAMINA_COST_ATTACK);
    }

    // ラッシュ攻撃（ダッシュ中）
    if (m_state == PlayerState::Rush)
    {
        // スペースを離した、またはスタミナ切れで終了
        if (!kb.Space || m_stats.stamina <= 0.0f)
        {
            m_state = PlayerState::Idle;
            
            m_attackCoolTime = ATTACK_COOLDOWN;
            return;
        }
        // Rush継続中の処理
        m_stats.UseStamina(STAMINA_COST_RUSH * elapsedTime);
        return;
    }

    //ダッシュ
    if (m_isDashing && kb.Space && m_stats.stamina >= STAMINA_REQ_RUSH
        && m_attackCoolTime <= 0.0f)
    {
        m_state = PlayerState::Rush;
        
        m_stateTimer = 0.0f;

		//攻撃処理
        if (m_attackManager)
            m_attackManager->Rush(this);
        {
            m_stats.UseStamina(STAMINA_COST_RUSH);
            
        }
    }
}

//-----------------------------------------------------------------
//スタミナの自然回復処理
//-----------------------------------------------------------------

void Player::UpdateStamina(float elapsedTime)
{
    // スタミナ自然回復
    m_stats.RecoverStamina(elapsedTime);
}

//-----------------------------------------------------------------
//ふらつき状態にする
//-----------------------------------------------------------------

void Player::ApplyDizzy()
{
    //すでにふらつき状態ならそのまま
    if (m_state == PlayerState::Dizzy) return;

    m_state = PlayerState::Dizzy;

    
    m_stateTimer = STATE_TIMER_DIZZY;

    //ダッシュなどの状態を強制解除
    m_isDashing = false;

    //慣性を少し残し、入力の速度をゼロにする
    m_attackCoolTime = 0.0f;

}

//-----------------------------------------------------------------
//プレイヤーにダメージ 攻撃を受けたときにHPを減らす
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

//----------------------------------------------------------
// リソース
//----------------------------------------------------------

void Player::CreateDeviceResources()
{
    auto device = m_deviceResources->GetD3DDevice();

    //------------------------------------------------------
    //ステートの作成
    //------------------------------------------------------

    m_states = std::make_unique<CommonStates>(device);

    
    
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
    AudioManager::GetInstance()->LoadSound("Attack", L"Resources/Sounds/P_近距離攻撃.wav");
    AudioManager::GetInstance()->LoadSound("Dash", L"Resources/Sounds/P_突進攻撃.wav");
    AudioManager::GetInstance()->LoadSound("Bullet", L"Resources/Sounds/P_E_遠距離攻撃.wav");

    //その他
    AudioManager::GetInstance()->LoadSound("Fall", L"Resources/Sounds/P_E_落水.wav");
    AudioManager::GetInstance()->LoadSound("Heal", L"Resources/Sounds/SE_回復.wav");

}

//----------------------------------------------------------
// 画面リソース
//----------------------------------------------------------

void Player::CreateWindowSizeResources(int /*width*/, int /*height*/)
{
}

//----------------------------------------------------------
//  魚の当たり判定を取得するための関数
//----------------------------------------------------------

ModelCollision* Player::GetCollision() const
{
    return m_collision.get();
}