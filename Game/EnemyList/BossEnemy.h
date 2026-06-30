
/**
 * @file   BossEnemy.h
 * @brief  ボスの管理を行うクラス
 * @author 國田知睦
 * @date   2026/06/24
 */

#pragma once

#include "pch.h"
#include "Game/Common/DeviceResources.h"
#include <SimpleMath.h>
#include <Model.h>
#include <CommonStates.h>

#include "Game/Collision/DisplayCollision.h"
#include <Game/Collision/ModelCollision.h>
#include <Game/GimmickList/Stage.h>

#include "Game/Effects/Particle.h"
#include <Game/Effects/Smoke.h>

#include "Game/ShadowRenderer/ShadowRenderer.h"

#include "Game/GimmickList/SlideBehavior.h"
#include "Game/Common/ObjectCharacter/CharacterBase.h"
#include "Game/EnemyList/EnemyAttackList/EnemyAttackPattern.h"
#include "Game/EnemyList/EnemyBase.h"

class EnemyRenderer;
class EnemyManager;


/// <summary>
/// 敵の種類　通常とボスで分けるため
/// 後々増やす
/// </summary>
enum class EnemyType
{
    Normal,
    Boss
};

class BossEnemy : public EnemyBase
{
private:

    //------------------------------------------------------
    //物理＆移動関連
    //------------------------------------------------------

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

    //プレイヤーとの距離（近すぎないように）
    static constexpr float STOP_DISTANCE = 5.0f;

    //------------------------------------------------------
    //ダメージ＆戦闘関連
    //------------------------------------------------------

    
    //回避が発動する連続被弾回数
    static constexpr int EVADE_HIT_THRESHOLD = 3;
    //回避が発動しなかったらリセット
    static constexpr int EVADE_NOHIT_THRESHOLD = 1;

    //回避状態の持続時間
    static constexpr float EVADE_DURATION = 0.5f;

    //何回で気絶
	static constexpr int STUN_HIT_THRESHOLD = 5;
    //気絶時間
	static constexpr float STUN_TIME = 100.0f;

    
    //------------------------------------------------------
    //描画＆エフェクト関連
    //------------------------------------------------------

    
    //着地煙の持続時間
    static constexpr float SMOKE_DURATION = 1.0f;
    //煙の初期サイズ
    static constexpr float SMOKE_BASE_SCALE = 3.0f;
    //煙の拡大サイズ
    static constexpr float SMOKE_SCALE = 30.0f;
    //煙の拡大速度
    static constexpr float SMOKE_GROWTH_RATE = 100.0f;
    //煙の発生位置
    static constexpr float SMOKE_Y_OFFSET = 0.5f;
    //落下したときの水しぶきの粒子数
    static constexpr float SPLASH_PARTICLE_COUNT = 50.0f;
    //接地エフェクト判定の高さ
    static constexpr float GROUND_LEVEL_THRESHOLD = 7.0f;

    //攻撃
	//近距離攻撃と突進の切り替え距離
    static constexpr float ATTACK_SWITCH_DISTANCE = 10.0f;
	//突進と回避の切り替え距離
    static constexpr float RUSH_SWITCH_DISTANCE = 20.0f;

    //近距離攻撃時間
    static constexpr float ATTACK_TIMER = 1.5f;
    //突進攻撃時間
    static constexpr float RUSH_TIMER = 7.0f;

    //攻撃の持続時間
    static constexpr float ATTACK_DURATION = 1.0f;

    //微小値
    static constexpr float VECTOR_EPSILON = 0.0001f;
    //コリジョンの透明度
    static constexpr float TRANSPARENCY = 0.15f;

public:

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
    void TakeDamage(float amount,PlayerAttackType type)override;

public:

    /// <summary>
    /// ボスのインスタンスを生成
    /// </summary>
    /// <param name="deviceResources">デバイスリソース</param>
    /// <param name="position">初期座標</param>
    /// <param name="hp">ボスの初期HP</param>
    /// <param name="speed">ボスの移動速度</param>
    /// <param name="displayCollision">表示用の当たり判定オブジェクト</param>
    BossEnemy(
        DX::DeviceResources* deviceResources,
        const DirectX::SimpleMath::Vector3& position,
        std::shared_ptr<DisplayCollision> displayCollision,
        EnemyType type = EnemyType::Boss
    );

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~BossEnemy() ;

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
    /// <param name="stage">地形情報取得用のステージポインタ</param>
    /// <param name="particle">パーティクル発生用マネージャー</param>
    void Update(
        float deltaTime,
        const DirectX::SimpleMath::Vector3& playerPosition,
        Stage* stage,
        Particle* particle,
        class EnemyManager* enemyManager = nullptr
    );

    /// <summary>
    /// ボスのモデルやエフェクト、デバッグ用コリジョンを描画
    /// </summary>
    /// <param name="context">デバイスコンテキスト</param>
    /// <param name="view">ビュー行列</param>
    /// <param name="proj">プロジェクション行列</param>
    void Render(
        ID3D11DeviceContext* context,
        const DirectX::SimpleMath::Matrix& view,
        const DirectX::SimpleMath::Matrix& proj
    )override;

    /// <summary>
    /// ボスが倒されたかどうか
    /// </summary>
    /// <returns>ボスのHPが0以下なら true</returns>
    bool IsDead() const { return m_hp <= 0; }

    /// <summary>
    /// 現在のワールド座標を取得
    /// </summary>
    /// <returns>敵の現在の座標</returns>
    DirectX::SimpleMath::Vector3 GetPosition() const { return m_position; }

    /// <summary>
    /// 当たり判定を取得
    /// </summary>
    /// <returns>敵の当たり判定</returns>
    ModelCollision* GetCollision() override { return m_collision.get(); }

    /// <summary>
    /// 攻撃を受けた際のノックバック力をボスに適用
    /// プレイヤーー＞敵
    /// </summary>
    /// <param name="direction">吹き飛ぶ方向ベクトル</param>
    /// <param name="power">ノックバックの強さ</param>
    void ApplyKnockback(const DirectX::SimpleMath::Vector3& direction, float power)override;
    
    /// <summary>
    /// 現在のHPを取得
    /// </summary>
    /// <returns>敵の体力</returns>
    float GetHP() const { return m_hp; }
    
    /// <summary>
    /// 最大HPを取得　UI表示用
    /// </summary>
    /// <returns>敵の最大の体力</returns>
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

    /// <summary>
	/// 敵のAI、物理挙動、状態遷移を更新
    /// </summary>
    /// <param name="dt">前フレームからの経過時間</param>
    /// <param name="playerPos">プレイヤーの現在座標</param>
    /// <param name="stage">地形情報取得用のステージポインタ</param>
    /// <param name="particle">パーティクル発生用マネージャー</param>
    /// <param name="enemyManager">敵の管理マネージャー</param>
    void UpdateAI(
        float dt,
        const DirectX::SimpleMath::Vector3& playerPos,
        Stage* stage,
        Particle* particle,
        EnemyManager* enemyManager
    )override;


    /// <summary>
    /// 敵の物理演算と移動
    /// </summary>
    /// <param name="stage">ステージのポインタ</param>
    /// <param name="dt">前フレームからの経過時間</param>
	/// <param name="particle">パーティクル発生用マネージャー</param>
    void UpdatePhysics(Stage* stage, float dt, Particle* particle = nullptr);
    
protected:

    /// <summary>
	/// 敵の落下の判定と処理
    /// </summary>
    /// <returns>落下判定の高さ</returns>
    float GetFallLimitY() const override { return FALL_LIMIT_Y; }

    /// <summary>
	/// 敵の落下ダメージを取得
    /// </summary>
    /// <returns>落下ダメージの量</returns>
    float GetFallDamage() const override { return EnemyBase::FALL_DAMAGE; }

    /// <summary>
	/// 敵の落下したときの水しぶきの粒子数を取得
    /// </summary>
    /// <returns>水しぶきの粒子数</returns>
    int GetSplashParticleCount() const override { return static_cast<int>(SPLASH_PARTICLE_COUNT); }

    /// <summary>
	/// 敵のリスポーン位置を取得
    /// </summary>
    /// <returns>リスポーン位置の座標</returns>
    DirectX::SimpleMath::Vector3 GetRespawnPosition() const override
    {
        // ボス専用の復帰座標があれば書き換えてください
        return DirectX::SimpleMath::Vector3(0.0f, RESPAWN_HEIGHT, 0.0f);
    }

    /// <summary>
    /// 敵が落下ダメージを受けるときの処理
    /// </summary>
    void ExecuteFallDamage() override
    {
        // ボスも同じようにダメージ関数を呼ぶ
        TakeDamage(GetFallDamage(), PlayerAttackType::None);
    }

private:
    DX::DeviceResources* m_deviceResources;

    DirectX::SimpleMath::Vector3 m_forward = { 0.0f,0.0f,0.0f };

    float m_rotationY = 0.0f;

    std::shared_ptr<DisplayCollision> m_displayCollision;
    std::unique_ptr<ModelCollision> m_collision;
    std::unique_ptr<DirectX::Model> m_model;
    std::unique_ptr<DirectX::CommonStates> m_states;

    SlideBehavior m_slideBehavior;
   
private:

    //プレイヤーの原因の状態
    EnemyState m_state = EnemyState::Opening;

    //モデルの切り替えている時間
    float m_stateTimer = 0.0f;
    
    //最後に食らった攻撃
    PlayerAttackType m_lastAttackType = PlayerAttackType::None;

    //連続で同じ攻撃かどうかカウントする
    int m_consecutiveHitCount = 0;

    //回避時の移動ベクトル
    DirectX::SimpleMath::Vector3 m_avoidVelocity = { 0,0,0 };

    //目標とする移動速度を保持
    DirectX::SimpleMath::Vector3 m_targetVelocity = DirectX::SimpleMath::Vector3::Zero;

    //プレイヤーの攻撃を数える
	int m_playerAttackCounter = 0;

	//気絶状態の揺れ用の回転角度
	float m_dizzyRotationY = 0.0f;

	//攻撃のクールダウンタイマー
	float m_attackCooldownTimer = 0.0f;

	//攻撃パターンのリスト
	std::vector<std::shared_ptr<EnemyAttackPattern>> m_attackPatterns;

	//近距離攻撃の持続時間
    float m_meleeTimer = 0.0f;
	//突進攻撃の持続時間
	float m_rushTimer = 0.0f;

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

    //描画
	std::unique_ptr<EnemyRenderer> m_renderer;

	//敵の種類
    EnemyType m_type = EnemyType::Boss;
};