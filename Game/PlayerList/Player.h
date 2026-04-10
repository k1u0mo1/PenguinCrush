
//プレイヤーのクラス

#pragma once
#include "pch.h"
#include <DirectXMath.h>
#include <memory>
#include "Game/Common/DeviceResources.h"

//プレイヤーのパラメータ管理
#include "PlayerStats.h"
//攻撃管理
#include "AttackList/AttackManager.h"
//衝突判定
#include "Game/Collision/DisplayCollision.h"
//ステージ
#include "Game/GimmickList/Stage.h"
//カメラ
#include "Game/Camera/PlayerCamera.h"
//エフェクト
#include "Game/Effects/Particle.h"
//音
#include "Game/SoundList/AudioManager.h"
//アニメーション
#include "Game/AnimatorList/Animator.h"
//影
#include "Game/ShadowRenderer/ShadowRenderer.h"
//滑る
#include "Game/GimmickList/SlideBehavior.h"


class Player
{
public:

    /// <summary>
    /// プレイヤーの現在のアクション状態
    /// </summary>
    enum class PlayerState
    {
        Idle,            //通常
        Attack,          //近距離攻撃
        Shoot,           //遠距離攻撃
        Rush,            //突進
        Dizzy            //攻撃後のふらつき
    };

private:

    //定数系
    
    //------------------------------------------------------
    //移動関連
    //------------------------------------------------------
    
    //通常の移動速度
    static constexpr float MOVE_SPEED = 15.0f;
    //ダッシュ時の速度
    static constexpr float DASH_SPEED = 30.0f;
    ////動きだしの加速
    //static constexpr float ACCELERATION_FORCE = 10.0f;
    ////加速から停止したときの摩擦
    //static constexpr float FRICTION_FORCE = 1.0f;
    //カメラへの追従する速度
    static constexpr float TURN_SPEED = 10.0f;

    //------------------------------------------------------
    //攻撃＆スタミナ関連
    //------------------------------------------------------

    //近距離攻撃のスタミナ消費量
    static constexpr float STAMINA_COST_ATTACK = 15.0f;
    //遠距離攻撃のスタミナ消費量
    static constexpr float STAMINA_COST_SHOOT = 10.0f;
    //突進攻撃（ダッシュ）中のフレームあたりのスタミナ消費量　
    static constexpr float STAMINA_COST_DASH = 10.0f;
    //突進攻撃（ダッシュ）中の継続スタミナ消費量
    static constexpr float STAMINA_COST_RUSH = 10.0f;

    //近距離攻撃で必要な最低スタミナ量
    static constexpr float STAMINA_REQ_ATTACK = 10.0f;
    //遠距離攻撃で必要な最低スタミナ量
    static constexpr float STAMINA_REQ_SHOOT = 5.0f;
    //突進攻撃で必要な最低スタミナ量
    static constexpr float STAMINA_REQ_RUSH = 30.0f;

    //攻撃のクールダウン
    static constexpr float ATTACK_COOLDOWN = 0.5f;
    //攻撃モーションの維持時間
    static constexpr float STATE_TIMER_ATTACK = 0.3f;
    //射撃モーションの維持時間
    static constexpr float STATE_TIMER_SHOOT = 0.2f;
    

    //------------------------------------------------------
    //システム＆物理関連
    //------------------------------------------------------

    //海へ落下したときのダメージ量
    static constexpr float FALL_DAMAGE = 50.0f;
    //落下する判定の高さ
    static constexpr float RESPAWN_THRESHOLD_Y = -5.0f;
    //足場補正の高さ
    static constexpr float STAGE_BOUNDARY_Y = -4.9f;
    //接地のめり込み補正
    static constexpr float GROUND_OFFSET_Y = 0.5f;

    //水しぶきの数
    static constexpr int SPLASH_OF_WATER = 30;
    
    //ふらつきの状態の継続時間
    static constexpr float STATE_TIMER_DIZZY = 2.0f;
    //ふらつきエフェクトの高さ
    static constexpr float DIZZY_EFFECT_OFFSET_Y = 2.5f;
    //ふらつきエフェクトの回転速度
    static constexpr float DIZZY_EFFECT_ROT_SPEED = 5.0f;
    //ふらつき時の揺れる速度
    static constexpr float DIZZY_SWAY_SPEED = 2.0f;
    //ふらつき時の揺れる最大角度
    static constexpr float DIZZY_SWAY_ANGLE = 0.26f;

public:

    /// <summary>
    /// プレイヤーインスタンスを生成
    /// </summary>
    /// <param name="deviceResources">デバイスリソース</param>
    /// <param name="collision">表示用の当たり判定オブジェクト</param>
    /// <param name="camera">プレイヤーを追従するカメラのポインタ</param>
    Player(
        DX::DeviceResources* deviceResources,
        std::shared_ptr<DisplayCollision> collision,
        PlayerCamera* camera
    );

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~Player() = default;
    
    /// <summary>
    /// プレイヤーの初期化
    /// </summary>
    /// <param name="hwnd">ウィンドウハンドル</param>
    /// <param name="width">画面の幅</param>
    /// <param name="height">画面の高さ</param>
    /// <param name="stage">足場のポインタ（傾きを渡す）</param>
    void Initalize(
        HWND hwnd, int width,
        int height,
        Stage* stage);

    /// <summary>
    /// プレイヤーの毎フレームの更新処理
    /// </summary>
    /// <param name="elapsedTime">前フレームからの経過時間</param>
    /// <param name="mouse">マウスの現在状態</param>
    /// <param name="mouseTracker">マウスのクリック判定用トラッカー</param>
    /// <param name="stage">地形判定用のステージポインタ</param>
    /// <param name="wave">波のポインタ（落下判定用）</param>
    /// <param name="particle">エフェクト発生用マネージャー</param>
    void Update(
        float elapsedTime,
        const DirectX::Mouse::State& mouse,
        const DirectX::Mouse::ButtonStateTracker& mouseTracker,
        Stage* stage,
        Wave* wave,
        Particle* particle);

    /// <summary>
    /// プレイヤーモデル、影、当たり判定の描画
    /// </summary>
    /// <param name="context">デバイスコンテキスト</param>
    /// <param name="view">ビュー行列</param>
    /// <param name="proj">プロジェクション行列</param>
    /// <param name="shadowRenderer">影のポインタ</param>
    void Render(ID3D11DeviceContext* context,
        DirectX::SimpleMath::Matrix& view,
        DirectX::SimpleMath::Matrix& proj,
        ShadowRenderer* shadowRenderer
    );

    /// <summary>
    /// どの攻撃かをもらう
    /// </summary>
    /// <param name="attackManager">攻撃マネージャーのポインタ</param>
    void SetAttackManager(AttackManager* attackManager) { m_attackManager = attackManager; }

    //攻撃関連

    /// <summary>
    /// 弾の弾数はあるのか？
    /// </summary>
    /// <returns>弾が撃てる状態なら true、撃てないなら false<</returns>
    bool CanShoot() const { return m_stats.ammo > 0; }
    
    /// <summary>
    /// 弾の発射
    /// </summary>
    void ShootBullet() { m_stats.UseAmmo(); }
    
    /// <summary>
    /// プレイヤーの座標を取得
    /// </summary>
    /// <returns>現在の座標</returns>
    DirectX::SimpleMath::Vector3 GetPosition() const { return m_position; }
    
    /// <summary>
    /// ステージの情報を取得
    /// </summary>
    /// <returns>m_stage</returns>
    const Stage* GetStage() const { return m_stage; }

    /// <summary>
    /// プレイヤーが突進中かどうか
    /// </summary>
    /// <returns>突進中　true　突進していない　false</returns>
    bool IsDashing() const { return m_isDashing; }

    //-----------------------------------------------------------
    
    /// <summary>
    /// リソース
    /// </summary>
    void CreateDeviceResources();
    
    /// <summary>
    /// 画面リソース
    /// </summary>
    /// <param name="width">画面の幅</param>
    /// <param name="height">画面の高さ</param>
    void CreateWindowSizeResources(int width, int height);
    //-----------------------------------------------------------

    /// <summary>
    /// プレイヤーの正面方向ベクトルを取得
    /// </summary>
    /// <returns>現在の正面方向</returns>
    DirectX::SimpleMath::Vector3 GetForward() const { return m_forward; }

    /// <summary>
    /// プレイヤーに追従するカメラを取得
    /// </summary>
    /// <returns>カメラのポインタ</returns>
    PlayerCamera* GetCamera() const { return m_camera; }

    /// <summary>
    /// デバイスリソース
    /// </summary>
    /// <returns>m_deviceResources</returns>
    DX::DeviceResources* GetDeviceResources() const { return m_deviceResources; }

    /// <summary>
    /// 当たり判定
    /// </summary>
    /// <returns>m_displayCollision</returns>
    std::shared_ptr<DisplayCollision> GetDisplayCollision() const { return m_displayCollision; }

    /// <summary>
    /// プレイヤーに外部からノックバック
    /// 敵ー＞プレイヤー
    /// </summary>
    /// <param name="direction">吹き飛ぶ方向</param>
    /// <param name="power">吹き飛ぶ力の強さ</param>
    void ApplyKnockback(const DirectX::SimpleMath::Vector3& direction, float power);

    /// <summary>
    /// 現在のプレイヤーの移動速度を計算して取得
    /// </summary>
    /// <returns>移動速度</returns>
    float GetMoveSpeed() const;

    //UIで使用する-----------------------------------

    /// <summary>
    /// 現在のHPを取得
    /// </summary>
    /// <returns>現在のHP</returns>
    float GetHP() const { return m_stats.hp; }

    /// <summary>
    /// 現在のスタミナを取得
    /// </summary>
    /// <returns>現在のスタミナ</returns>
    float GetStamina() const { return m_stats.stamina; }

    /// <summary>
    /// 弾数を取得用
    /// </summary>
    /// <returns>現在の弾数</returns>
    int GetAmmo() const { return m_stats.ammo; }
    
    /// <summary>
    /// 最大弾数を取得
    /// </summary>
    /// <returns>最大弾数</returns>
    int GetMaxAmmo() const { return m_stats.ammo_Max; }

    //-------------------------------------------------

    /// <summary>
    /// プレイヤーの体力を回復
    /// </summary>
    /// <param name="amount">回復する量</param>
    void Heal(float amount) { m_stats.Heal(amount); }

    
    /// <summary>
    /// 魚の当たり判定
    /// </summary>
    /// <returns>魚の当たり判定</returns>
    ModelCollision* GetCollision() const { return m_collision.get(); }
    
    /// <summary>
    /// 弾薬を補充
    /// </summary>
    /// <param name="value">補充する弾数</param>
    void AddAmmo(int value)
    {
        // 弾を増やす
        m_stats.ammo = std::min(m_stats.ammo + value, m_stats.ammo_Max);

        //効果音
        AudioManager::GetInstance()->Play("Reload");
    }

    /// <summary>
    /// プレイヤーにダメージ
    /// </summary>
    /// <param name="amount">受けるダメージ量</param>
    void TakeDamage(float amount);

    /// <summary>
    /// プレイヤーがやられているかを判定
    /// </summary>
    /// <returns>やられた　HPが０以下のときtrue</returns>
    bool IsDead()const { return m_stats.hp <= 0.0f; }

    /// <summary>
    /// 当たり判定を強制的に動かす
    /// </summary>
    /// <param name="pos">設定するワールド座標</param>
    void SetPosition(const DirectX::SimpleMath::Vector3& pos) { m_position = pos; }

    /// <summary>
    /// プレイヤーの移動、ダッシュ、慣性、地形に沿った高さ補正の処理
    /// </summary>
    /// <param name="elapsedTime">前フレームからの経過時間</param>
    /// <param name="stage">>足場情報を取得するためのステージポインタ</param>
    /// <param name="particle">突進時のエフェクト生成用マネージャー</param>
    void HandleMovement(float elapsedTime, Stage* stage, Particle* particle);

    /// <summary>
    /// マウスとキーボード入力に応じた攻撃の処理
    /// </summary>
    /// <param name="elapsedTime">前フレームからの経過時間</param>
    /// <param name="mouse">マウスの現在状態</param>
    /// <param name="mouseTracker">マウスクリックのエッジ検出用トラッカー</param>
    /// <param name="kb">キーボードの現在状態</param>
    void HandleAttack(
        float elapsedTime,
        const DirectX::Mouse::State& mouse,
        const DirectX::Mouse::ButtonStateTracker& mouseTracker,
        const DirectX::Keyboard::State& kb
    );
    
    /// <summary>
    /// スタミナの自然回復処理
    /// </summary>
    /// <param name="elapsedTime">前フレームからの経過時間</param>
    void UpdateStamina(float elapsedTime);
    
    /// <summary>
    /// ふらつき状態にする
    /// </summary>
    void ApplyDizzy();

private:

    DX::DeviceResources* m_deviceResources;

    std::shared_ptr<DisplayCollision> m_displayCollision;

    //衝突判定オブジェクトの追加
    std::unique_ptr<ModelCollisionOrientedBox> m_collision;

    AttackManager* m_attackManager = nullptr;

    PlayerStats m_stats;

    //カメラ
    PlayerCamera* m_camera=nullptr;

    std::unique_ptr<DirectX::CommonStates> m_states;
    std::shared_ptr<DirectX::Model> m_model;

    DirectX::SimpleMath::Vector3 m_position = { 0,0,0 };
    DirectX::SimpleMath::Vector3 m_forward = { 0,0,1 };

    bool m_isDashing = false;
    
    DirectX::SimpleMath::Vector3 m_velocity = { 0,0,0 };
    
    //重力
    float m_gravity = -100.8f;

    // Optional: スライド移動用
    DirectX::SimpleMath::Vector3 m_slideVelocity = { 0,0,0 };
    //滑る慣性(残り)
    //DirectX::SimpleMath::Vector3 m_slidingInertia;
     
    SlideBehavior m_slideBehavior;

    //ノックバック用
    DirectX::SimpleMath::Vector3 m_knockbackVelocity = { 0, 0, 0 };
    //ノックバック持続タイマー
    float m_knockbackTimer = 0.0f;
    //速度減衰率
    const float KNOCKBACK_DRAG = 10.0f;

    //攻撃の連射防止用タイマー
    float m_attackCoolTime = 0.0f;

private:

    //アニメーション　まだ実装未定
    std::unique_ptr<Animator> m_animator;

    //プレイヤーの原因の状態
    PlayerState m_state = PlayerState::Idle;
    //モデルの切り替えている時間
    float m_stateTimer = 0.0f;

    //モデルを複数保持する
    std::shared_ptr<DirectX::Model> m_modelIdle;
    std::shared_ptr<DirectX::Model> m_modelAttack;
    std::shared_ptr<DirectX::Model> m_modelShoot;
    std::shared_ptr<DirectX::Model> m_modelRush;

    //ふらついているときの素材
    std::shared_ptr<DirectX::Model> m_materialDizzy;

    //現在の描画に使うモデルを指すポインタ
    DirectX::Model* m_currentModel = nullptr;

    //ふらつきエフェクトの現在の回転角度
    float m_dizzyRotationY = 0.0f;

private:

    //影用のプリミティブ　板
    std::unique_ptr<DirectX::GeometricPrimitive> m_shadowQuad;

    //影のエフェクト
    std::unique_ptr<DirectX::BasicEffect> m_shadowEffect;

    //影のテクスチャ
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_shadowTexture;

    //入力レイアウト
    Microsoft::WRL::ComPtr<ID3D11InputLayout>    m_shadowInputLayout;

    //影
    //std::unique_ptr<ShadowRenderer> m_shadowRenderer;

    Stage* m_stage = nullptr;


};
