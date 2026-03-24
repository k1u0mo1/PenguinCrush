
//敵のクラス
#pragma once

#include "pch.h"
#include "Common/DeviceResources.h"
#include <SimpleMath.h>
#include <Model.h>
#include <CommonStates.h>

#include "Collision/DisplayCollision.h"
#include <Collision/ModelCollision.h>
#include <GimmickList/Stage.h>

#include "Effects/Particle.h"
#include <Effects/Smoke.h>


#include "ShadowRenderer/ShadowRenderer.h"

class BossEnemy
{
private:

    //------------------------------------------------------
    //物理＆移動関連
    //------------------------------------------------------

    //重力加速度
    static constexpr float GRAVITY_FORCE = -100.8f;
    //ノックバックの速度減衰率
    static constexpr float KNOCKBACK_DRAG = 10.0f;
    //復活する時の判定の高さ
    static constexpr float FALL_LIMIT_Y = -5.0f;
    //接地する足場の位置の高さ
    static constexpr float STAGE_BOUNDARY_Y = -4.9f;
    //接地時の埋まり防止の高さ
    static constexpr float GROUND_HEIGHT_OFFSET = 0.5f;
    //落下からの復帰高度
    static constexpr float RESPAWN_HEIGHT = 10.0f;
    //プレイヤーを追尾を開始する距離
    static constexpr float PLAYER_DISTANCE_THRESHOLD = 2.0f;

    //------------------------------------------------------
    //ダメージ＆戦闘関連
    //------------------------------------------------------

    //海に落ちたときに受けるダメージ量
    static constexpr float FALL_DAMAGE = 250.0f;
    //回避が発動する連続被弾回数
    static constexpr int EVADE_HIT_THRESHOLD = 3;
    //回避状態の持続時間
    static constexpr float EVADE_DURATION = 0.5f;
    //プレイヤーからの攻撃のノックバックの力
    static constexpr float KNOCKBACK_POWER_SCALE = 10.0f;
    //ノックバックの持続時間
    static constexpr float KNOCKBACK_DURATION = 0.15f;

    //------------------------------------------------------
    //描画＆エフェクト関連
    //------------------------------------------------------

    //モデルの描画サイズ
    static constexpr float MODEL_RENDER_SCALE = 1.7f;
    //着地煙の持続時間
    static constexpr float SMOKE_DURATION = 1.0f;
    //煙の初期サイズ
    static constexpr float SMOKE_BASE_SCALE = 3.0f;
    //煙の拡大速度
    static constexpr float SMOKE_GROWTH_RATE = 100.0f;
    //煙の発生位置
    static constexpr float SMOKE_Y_OFFSET = 0.5f;
    //落下したときの水しぶきの粒子数
    static constexpr float SPLASH_PARTICLE_COUNT = 50.0f;
    //接地エフェクト判定の高さ
    static constexpr float GROUND_LEVEL_THRESHOLD = 7.0f;

public:

    /// <summary>
    /// ボスのアクション状態
    /// </summary>
    enum class EnemyState
    {
        Opening,         //登場演出（急に動かないように）
        Loading,         //着地後に止まるように
        Idle,            //通常
        Attack,          //近距離攻撃
        Shoot,           //遠距離攻撃
        Rush,            //突進
        Avoid            //回避状態
    };

    /// <summary>
    /// プレイヤーの攻撃タイプ
    /// 判別するため
    /// </summary>
    enum class PlayerAttackType 
    {
        None,
        Attack, // 近接
        Shoot,  // 弾
        Rush    // 突進
    };

    /// <summary>
    /// ボスの状態を変更、その状態を維持するタイマーを設定
    /// </summary>
    /// <param name="state">変更する状態</param>
    /// <param name="duration">状態を維持する時間</param>
    void SetState(EnemyState state, float duration);

    /// <summary>
    /// ボスの現在の状態を取得
    /// </summary>
    /// <returns>現在のEnemyState　m_state</returns>
    EnemyState GetState() const { return m_state; }

    /// <summary>
    /// ボスにダメージを与え、連続攻撃による回避判定
    /// </summary>
    /// <param name="amount">ダメージ量</param>
    /// <param name="type">プレイヤーから受けた攻撃の種類</param>
    void TakeDamage(float amount,PlayerAttackType type);

public:

    /// <summary>
    /// ボスのインスタンスを生成
    /// </summary>
    /// <param name="deviceResources">デバイスリソース</param>
    /// <param name="position">初期ワールド座標</param>
    /// <param name="hp">初期HP</param>
    /// <param name="speed">移動速度</param>
    /// <param name="displayCollision">デバッグ用当たり判定描画クラス</param>
    BossEnemy(
        DX::DeviceResources* deviceResources,
        const DirectX::SimpleMath::Vector3& position,
        float hp,
        float speed,
        std::shared_ptr<DisplayCollision> displayCollision
    );

    //デストラクタ
    ~BossEnemy() = default;

    /// <summary>
    /// モデルやエフェクト、サウンドなどの初期化処理
    /// </summary>
    void Initialize();

    /// <summary>
    /// ボスのAI、物理挙動、状態遷移を更新
    /// 追尾、重力・スライド
    /// </summary>
    /// <param name="deltaTime">前フレームからの経過時間</param>
    /// <param name="playerPosition">プレイヤーの現在座標</param>
    /// <param name="playerSpeed">プレイヤーの現在の移動速度</param>
    /// <param name="stage">地形情報取得用のステージポインタ</param>
    /// <param name="particle">パーティクル発生用マネージャー</param>
    void Update(
        float deltaTime,
        const DirectX::SimpleMath::Vector3& playerPosition,
        float playerSpeed,
        Stage* stage,
        Particle* particle
    );

    /// <summary>
    /// ボスのモデルやエフェクト、デバッグ用コリジョンを描画
    /// </summary>
    /// <param name="context"></param>
    /// <param name="view"></param>
    /// <param name="proj"></param>
    void Render(
        ID3D11DeviceContext* context,
        const DirectX::SimpleMath::Matrix& view,
        const DirectX::SimpleMath::Matrix& proj
    );

    /// <summary>
    /// ボスが倒されたかどうか
    /// </summary>
    /// <returns>ボスのHPが0以下なら true</returns>
    bool IsDead() const { return m_hp <= 0; }

    /// <summary>
    /// 現在のワールド座標を取得
    /// </summary>
    /// <returns>m_position</returns>
    DirectX::SimpleMath::Vector3 GetPosition() const { return m_position; }

    /// <summary>
    /// 当たり判定を取得
    /// </summary>
    /// <returns>m_collision</returns>
    ModelCollision* GetCollision() const { return m_collision.get(); }

    /// <summary>
    /// 攻撃を受けた際のノックバック力をボスに適用
    /// プレイヤーー＞敵
    /// </summary>
    /// <param name="direction">吹き飛ぶ方向ベクトル</param>
    /// <param name="power">ノックバックの強さ</param>
    void ApplyKnockback(const DirectX::SimpleMath::Vector3& direction, float power);
    
    /// <summary>
    /// 現在のHPを取得
    /// </summary>
    /// <returns>m_hp</returns>
    float GetHP() const { return m_hp; }
    
    /// <summary>
    /// 最大HPを取得　UI表示用
    /// </summary>
    /// <returns>m_maxHp</returns>
    float GetMaxHP() const { return m_maxHp; }

    /// <summary>
    /// ボスの位置を強制的に設定
    /// </summary>
    /// <param name="pos">設定するワールド座標</param>
    void SetPosition(const DirectX::SimpleMath::Vector3& pos) { m_position = pos; }

    /// <summary>
    /// ボスの体の向きを設定
    /// Y軸回転
    /// </summary>
    /// <param name="angle">Y軸の回転角度</param>
    void SetRotationY(float angle) { m_rotationY = angle; }

    
private:
    DX::DeviceResources* m_deviceResources;

    //パラメータ
    float m_hp;
    float m_maxHp;

    float m_speed;


    DirectX::SimpleMath::Vector3 m_position;

    DirectX::SimpleMath::Vector3 m_forward = { 0.0f,0.0f,0.0f };

    float m_rotationY = 0.0f;

    std::shared_ptr<DisplayCollision> m_displayCollision;
    std::unique_ptr<ModelCollision> m_collision;
    std::unique_ptr<DirectX::Model> m_model;
    std::unique_ptr<DirectX::CommonStates> m_states;

    //重力
    DirectX::SimpleMath::Vector3 m_velocity = DirectX::SimpleMath::Vector3::Zero;
    float m_gravity = -100.8f;

    //ノックバック用
    DirectX::SimpleMath::Vector3 m_knockbackVelocity = { 0, 0, 0 };

    //ノックバック持続タイマー
    float m_knockbackTimer = 0.0f;

   
private:

    //プレイヤーの原因の状態
    EnemyState m_state = EnemyState::Opening;
    //モデルの切り替えている時間
    float m_stateTimer = 0.0f;

    //モデルを複数保持する
    std::shared_ptr<DirectX::Model> m_modelIdle;
    std::shared_ptr<DirectX::Model> m_modelAttack;
    std::shared_ptr<DirectX::Model> m_modelShoot;
    std::shared_ptr<DirectX::Model> m_modelRush;
    //回避時
    std::unique_ptr<DirectX::Model> m_modelAwakening;

    //現在の描画に使うモデルを指すポインタ
    DirectX::Model* m_currentModel = nullptr;

    //最後に食らった攻撃
    PlayerAttackType m_lastAttackType = PlayerAttackType::None;

    //連続で同じ攻撃かどうかカウントする
    int m_consecutiveHitCount = 0;

    //回避時の移動ベクトル
    DirectX::SimpleMath::Vector3 m_avoidVelocity = { 0,0,0 };


private:

    //着地したら煙を出す
    //エフェクト
    std::unique_ptr<Smoke> m_smokeEffect;
    //煙が出ているか
    bool m_isSmokeActive = false;

    //煙の経過時間
    float m_smokeTimer;
    //煙の発生位置
    DirectX::SimpleMath::Vector3 m_smokePosition;

    //着地を覚える
    bool m_isGroundPrev;

    //着地を判定する
    bool m_isGroundNow;

    //一度だけ
    bool m_isLandingEffectDone = false;

};