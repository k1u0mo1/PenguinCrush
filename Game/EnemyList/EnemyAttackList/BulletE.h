// BulletE.h
#pragma once

#include "pch.h"

#include "Game/Common/DeviceResources.h"
#include "Game/Common/StepTimer.h"

#include "Game/Camera/PlayerCamera.h"
#include "Game/Collision/DisplayCollision.h"
#include "Game/Collision/ModelCollision.h"

#include "Game/GimmickList/Stage.h"

#include "Game/EnemyList/EnemyAttackList/EnemyAttackBase.h"

//影
class ShadowRenderer;

class BulletE : public EnemyAttackBase
{
private:

    //------------------------------------------------------
    //攻撃関連
    //------------------------------------------------------

    //遠距離攻撃のダメージ量
    static constexpr float ATTACK_DAMAGE = 10.0f;
    //ノックバックの力
    static constexpr float KNOCKBACK_POWER = 10.0f;
    //移動速度
    static constexpr float SPEED_MULTIPLIER = 20.0f;

    //------------------------------------------------------
    //描画＆サイズ関連
    //------------------------------------------------------

    //モデルのY軸の初期回転角
    static constexpr float MODEL_ROTATION_Y = 180.0f;

public:

    /// <summary>
    /// 敵が発射する弾を生成
    /// </summary>
    /// <param name="deviceResources">デバイスリソース</param>
    /// <param name="pos">発射位置</param>
    /// <param name="dir">発射する方向ベクトル</param>
    /// <param name="displayCollision">デバッグ描画用</param>
    /// <param name="model">弾の描画用モデル</param>
    /// <param name="speed">弾の基本速度</param>
    BulletE(DX::DeviceResources* deviceResources,
        const DirectX::SimpleMath::Vector3& pos,
        const DirectX::SimpleMath::Vector3& dir,
        std::shared_ptr<DisplayCollision> displayCollision,
        std::shared_ptr<DirectX::Model> model,
        float speed = 1.0f);

    /// <summary>
    /// 弾の移動と寿命を更新
    /// </summary>
    /// <param name="deltaTime">前フレームからの経過時間</param>
    void Update(float deltaTime) override;

    /// <summary>
    /// 弾の移動と寿命を更新と地形との衝突判定
    /// </summary>
    /// <param name="deltaTime">前フレームからの経過時間</param>
    /// <param name="stage">地形の高さを取得するためのステージポインタ</param>
    void Update(float deltaTime, const Stage* stage);

    /// <summary>
    /// 弾のモデルを描画
    /// </summary>
    /// <param name="context">デバイスコンテキスト</param>
    /// <param name="view">ビュー行列</param>
    /// <param name="proj">プロジェクション行列</param>
    void Render(ID3D11DeviceContext* context,
        const DirectX::SimpleMath::Matrix& view,
        const DirectX::SimpleMath::Matrix& proj
    );

    /// <summary>
    /// 弾が現在有効かどうか
    /// </summary>
    /// <returns>生存フラグと寿命内であればtrue</returns>
    bool IsAlive() const;
    
    /// <summary>
    /// 弾を強制的に消滅状態
    /// </summary>
    void BulletKill();

    // コピー禁止
    BulletE(const BulletE&) = delete;
    BulletE& operator=(const BulletE&) = delete;

    // ムーブ許可
    BulletE(BulletE&&) = default;
    BulletE& operator=(BulletE&&) = default;
    
    /// <summary>
    /// 当たり判定の範囲
    /// </summary>
    /// <returns>box</returns>
    DirectX::BoundingBox GetBoundingBox() const;
    
    /// <summary>
    /// 弾の寿命が尽きる、または消滅フラグが立ったかどうか
    /// </summary>
    /// <returns>消滅すべき状態ならtrue</returns>
    bool IsDead() const override { return !m_isAlive || (m_lifetime >= MAX_LIFETIME); }

    /// <summary>
    /// 攻撃を強制的に終了
    /// </summary>
    void SetDead() override { m_isAlive = false; }

    /// <summary>
    /// 攻撃が当たった時のノックバック力を取得
    /// 敵ー＞プレイヤー
    /// </summary>
    /// <returns>ノックバック力</returns>
    float GetKnockbackPower() const override { return KNOCKBACK_POWER; }

    /// <summary>
    /// 当たり判定を取得
    /// </summary>
    /// <returns>モデルコリジョンのポインタ　m_collision.get()</returns>
    ModelCollision* GetCollision() const override { return m_collision.get(); }
    
    /// <summary>
    /// 攻撃判定の現在のワールド座標を取得
    /// </summary>
    /// <returns>現在の座標　m_position</returns>
    DirectX::SimpleMath::Vector3 GetPosition() const override { return m_position; }

    /// <summary>
    /// 攻撃の進行方向ベクトルを取得
    /// </summary>
    /// <returns>進行方向ベクトル　m_forward</returns>
    DirectX::SimpleMath::Vector3 GetForward() const override { return m_direction; }
   
    /// <summary>
    /// 遠距離攻撃のダメージ量を取得
    /// </summary>
    /// <returns>ダメージ量</returns>
    float GetDamage()const override { return ATTACK_DAMAGE; }

private:

    DirectX::SimpleMath::Vector3 m_position;

    DirectX::SimpleMath::Vector3 m_direction;

    float m_speed;

    float m_lifetime;

    static constexpr float MAX_LIFETIME = 20.0f;

    DX::DeviceResources* m_deviceResources;

    //足元から原点の高さ
    float m_bulletHeightOffset;

    bool m_isAlive;

    //  射影行列
    DirectX::SimpleMath::Matrix m_proj;

    std::unique_ptr<DirectX::CommonStates> m_states;
    //std::unique_ptr<DirectX::Model> m_bulletModel;
    std::shared_ptr<DirectX::Model> m_bulletModel;

    std::unique_ptr<ModelCollisionOrientedBox> m_collision;

    //※参照を持たせる
    //DisplayCollision* m_displayCollision = nullptr;
    std::shared_ptr<DisplayCollision> m_displayCollision;
};
