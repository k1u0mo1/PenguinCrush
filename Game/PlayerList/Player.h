
/**
 * @file   Player.h
 * @brief  プレイヤーキャラクターの制御・パラメータ管理を行うクラス
 * @author 國田知睦
 * @date   2026/07/17
 */

#pragma once
#include "pch.h"
#include <DirectXMath.h>
#include <memory>
#include "Game/Common/DeviceResources.h"
#include "PlayerStats.h"
#include "Game/Collision/DisplayCollision.h"
#include "Game/GimmickList/SlideBehavior.h"
#include "Game/Common/ObjectCharacter/CharacterBase.h"

class PlayerRenderer;
class Stage;
class PlayerCamera;
class Particle;
class WaveManager;
class ShadowRenderer;
class AttackManager;
class Animator;
class ModelCollision;
class ModelCollisionOrientedBox;

class Player : public CharacterBase
{
public:

    /// <summary>
    /// プレイヤーの現在のアクション状態
    /// </summary>
    enum class PlayerState
    {
        Idle,            //通常
        Attack,          //近距離攻撃
        Rush,            //突進
        Dizzy            //攻撃後のふらつき
    };

private:

    //プレイヤーの初期座標
	static constexpr float PLAYER_START_POSITION_X = -10.0f;
	static constexpr float PLAYER_START_POSITION_Y =   0.3f;
	static constexpr float PLAYER_START_POSITION_Z = -10.0f;

    //移動関連
	//通常移動の速度
    static constexpr float MOVE_SPEED = 15.0f;
	//ダッシュの速度
    static constexpr float DASH_SPEED = 20.0f;
	//移動の減速率
    static constexpr float TURN_SPEED = 10.0f;

    //地形関連
	//地面からの高さのオフセット　
    static constexpr float GROUND_OFFSET_Y = 0.5f;
	//落下しているとみなすY座標
    static constexpr float STAGE_BOUNDARY_Y = -5.0f;
	//リスポーンするY座標
    static constexpr float RESPAWN_THRESHOLD_Y = -15.0f;

    //ダメージ・効果
	//ダメージを受けたときの無敵時間
    static constexpr float DIZZY_EFFECT_ROT_SPEED = 2.0f;
	//水しぶきのエフェクトの高さ
    static constexpr int SPLASH_OF_WATER = 50;
	//リスポーンする高さ
	static constexpr float RESPAWN_HEIGHT = 10.0f;

    //状態タイマー・クールタイム
	//攻撃後のふらつき時間
    static constexpr float STATE_TIMER_DIZZY = 2.0f;
	//攻撃のモーション時間
    static constexpr float STATE_TIMER_ATTACK = 0.4f;
	//クールタイム
    static constexpr float ATTACK_COOLDOWN = 0.5f;

    //スタミナ関連
	//近距離攻撃で必要な最低スタミナ量
    static constexpr float STAMINA_REQ_ATTACK = 10.0f;
	//突進攻撃で必要な最低スタミナ量
    static constexpr float STAMINA_REQ_RUSH = 20.0f;
	//攻撃に必要なスタミナ
    static constexpr float STAMINA_COST_ATTACK = 15.0f;
	//ダッシュに必要なスタミナ
    static constexpr float STAMINA_COST_DASH = 15.0f;
	//突進に必要なスタミナ
    static constexpr float STAMINA_COST_RUSH = 5.0f;
	
    //ノックバック関連
    //吹っ飛ぶ強さ
    static constexpr float KNOCKBACK_POWER_SCALE = 10.0f;
    //吹き飛ぶ時に上方向への力
    static constexpr float KNOCKBACK_UP_FORCE = 50.0f;
    //ノックバックの持続時間
    static constexpr float KNOCKBACK_DURATION = 0.2f;

    //ダッシュ時の高さ
	static constexpr float DASH_HEIGHT = 0.5f;
	//ダッシュ時のエフェクトの数
	static constexpr int DASH_EFFECT_NUM = 20;
	//ダッシュエフェクトのサイズ
	static constexpr float DASH_EFFECT_SIZE = 0.6f;

	//当たり判定の線の太さ
    static constexpr float COLLISION_LINE_THICKNESS = 0.1f;

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
    ~Player();

    /// <summary>
    /// プレイヤーの初期化
    /// </summary>
    /// <param name="hwnd">ウィンドウハンドル</param>
    /// <param name="width">画面の幅</param>
    /// <param name="height">画面の高さ</param>
    /// <param name="stage">足場のポインタ（傾きを渡す）</param>
    void Initialize(
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
        WaveManager* waveManager,
        Particle* particle);

    /// <summary>
    /// プレイヤーモデル、影、当たり判定の描画
    /// </summary>
    /// <param name="context">デバイスコンテキスト</param>
    /// <param name="view">ビュー行列</param>
    /// <param name="proj">プロジェクション行列</param>
    /// <param name="shadowRenderer">影のポインタ</param>
    void Render(ID3D11DeviceContext* context,
        const DirectX::SimpleMath::Matrix& view,
        const DirectX::SimpleMath::Matrix& proj,
        ShadowRenderer* shadowRenderer
    );

    /// <summary>
    /// どの攻撃かをもらうか判別
    /// </summary>
    /// <param name="attackManager">攻撃マネージャーのポインタ</param>
    void SetAttackManager(AttackManager* attackManager) { m_attackManager = attackManager; }

    /// <summary>
    /// ステージの情報を取得
    /// </summary>
    /// <returns>m_stage</returns>
    const Stage* GetStage() const { return m_stage; }

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
    /// 最大のスタミナを取得
    /// </summary>
    /// <returns>最大のスタミナ</returns>
    float GetMaxStamina() const { return m_stats.stamina_Max; }

    //-------------------------------------------------

    /// <summary>
    /// 魚の当たり判定
    /// </summary>
    /// <returns>魚の当たり判定</returns>
    ModelCollision* GetCollision() const;

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
    /// プレイヤーに外部からノックバック
    /// 敵ー＞プレイヤー
    /// </summary>
    /// <param name="direction">吹き飛ぶ方向</param>
    /// <param name="power">吹き飛ぶ力の強さ</param>
    void ApplyKnockback(const DirectX::SimpleMath::Vector3& direction, float power);

    /// <summary>
    /// プレイヤーの体力を回復
    /// </summary>
    /// <param name="amount">回復する量</param>
    void Heal(float amount) { m_stats.Heal(amount); }
    
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

	/// <summary>
	/// プレイヤーの落下判定のY座標を取得
	/// </summary>
	/// <returns>落下判定のY座標</returns>
	float GetFallLimitY() const override { return STAGE_BOUNDARY_Y; }

	/// <summary>
	/// プレイヤーのリスポーンするY座標を取得
	/// </summary>
	/// <returns>リスポーンするY座標</returns>
	float GetFallDamage() const override { return CharacterBase::FALL_DAMAGE; }

    /// <summary>
	/// プレイヤーのリスポーン位置を取得
    /// </summary>
    /// <returns>リスポーン位置</returns>
    DirectX::SimpleMath::Vector3 GetRespawnPosition() const override
    {
        return DirectX::SimpleMath::Vector3(0.0f, RESPAWN_HEIGHT, 0.0f);
	}

    /// <summary>
    /// 落下ダメージを適用 ダメージ調整はCharacterBase
    /// </summary>
    void ExecuteFallDamage() override 
    {
        TakeDamage(GetFallDamage());
    }

private:
    //デバイスリソース
    DX::DeviceResources* m_deviceResources;
    //当たり判定の表示用クラス
    std::shared_ptr<DisplayCollision> m_displayCollision;
    //衝突判定オブジェクトの追加
    std::unique_ptr<ModelCollisionOrientedBox> m_collision;

    //攻撃マネージャーのポインタ
    AttackManager* m_attackManager = nullptr;
    //プレイヤーのパラメータ
    PlayerStats m_stats;
    //カメラ
    PlayerCamera* m_camera = nullptr;
    
    std::unique_ptr<DirectX::CommonStates> m_states;
    std::shared_ptr<DirectX::Model> m_model;

    //プレイヤーの正面方向ベクトル
    DirectX::SimpleMath::Vector3 m_forward = { 0,0,1 };

    //プレイヤーが突進中か
    bool m_isDashing = false;

    //Optional: スライド移動用
    DirectX::SimpleMath::Vector3 m_slideVelocity = { 0,0,0 };
    SlideBehavior m_slideBehavior;

    //プレイヤーの現在の回転行列を保持する
    DirectX::SimpleMath::Matrix m_rotationMatrix = DirectX::SimpleMath::Matrix::Identity;
    //攻撃の連射防止用タイマー
    float m_attackCoolTime = 0.0f;

private:

	//プレイヤーの描画クラス
    std::unique_ptr<PlayerRenderer> m_renderer;
    //プレイヤーの原因の状態
    PlayerState m_state = PlayerState::Idle;
    //モデルの切り替えている時間
    float m_stateTimer = 0.0f;
    
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
    Stage* m_stage = nullptr;
};
