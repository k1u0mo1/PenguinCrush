// AttackE.h
#pragma once

#include "pch.h"

#include "Common/DeviceResources.h"
#include "Common/StepTimer.h"

#include "Camera/PlayerCamera.h"

#include "Collision/DisplayCollision.h"

#include "Collision/ModelCollision.h"

#include "EnemyList/EnemyAttackList/EnemyAttackBase.h"

class AttackE : public EnemyAttackBase
{
private:

    //------------------------------------------------------
    //攻撃関連
    //------------------------------------------------------

    //近距離攻撃のダメージ量
    static constexpr float ATTACK_DAMAGE = 20.0f;
    //ノックバックの力
    static constexpr float KNOCKBACK_POWER = 20.0f;
    //判定の移動速度
    static constexpr float ATTACK_SPEED = 10.0f;

    //------------------------------------------------------
    //座標＆サイズ関連
    //------------------------------------------------------

    //発生位置の高さ
    static constexpr float SPAWN_HEIGHT_OFFSET = 0.5f;
    //プレイヤーとの発生距離
    static constexpr float SPAWN_DISTANCE = 1.0f;
    //当たり判定のサイズ
    static constexpr float DEFAULT_BOUNDING_SIZE = 0.3f;


public:

    /// <summary>
    /// 短時間だけ発生する攻撃判定を生成
    /// </summary>
    /// <param name="deviceResources">デバイスリソース</param>
    /// <param name="playerPos">プレイヤーの座標</param>
    /// <param name="forward">攻撃を発生させる方向</param>
    /// <param name="displayCollision">デバッグ用の当たり判定描画クラス</param>
    AttackE(
        DX::DeviceResources* deviceResources,
        const DirectX::SimpleMath::Vector3& playerPos,
        const DirectX::SimpleMath::Vector3& forward,
        std::shared_ptr<DisplayCollision> displayCollision);

    /// <summary>
    /// 攻撃判定の移動と寿命を更新
    /// </summary>
    /// <param name="deltaTime">前フレームからの経過時間</param>
    void Update(float deltaTime)override;

    /// <summary>
    /// 攻撃判定を描画
    /// </summary>
    /// <param name="context"></param>
    /// <param name="view"></param>
    /// <param name="proj"></param>
    void Render(
        ID3D11DeviceContext* context,
        const DirectX::SimpleMath::Matrix& view,
        const DirectX::SimpleMath::Matrix& proj)override;

    /// <summary>
    /// 攻撃の寿命が尽きたかどうか
    /// </summary>
    /// <returns>寿命（MAX_LIFETIME）を超えていれば</returns>
    bool IsDead() const override { return m_lifetime >= MAX_LIFETIME; } 

    /// <summary>
    /// 攻撃を強制的に終了
    /// </summary>
    void SetDead() override { m_lifetime = MAX_LIFETIME; } 
    
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
    DirectX::SimpleMath::Vector3 GetForward() const override { return m_forward; }

    /// <summary>
    /// 近距離攻撃のダメージ量を取得
    /// </summary>
    /// <returns>ダメージ量</returns>
    float GetDamage() const override { return ATTACK_DAMAGE; }

private:

    //範囲
    DirectX::BoundingBox GetBoundingBox() const;

private:

    DX::DeviceResources* m_deviceResources;

    std::shared_ptr<DirectX::Model> m_attackModel;

    DirectX::SimpleMath::Vector3 m_position;

    DirectX::SimpleMath::Vector3 m_forward;

    std::unique_ptr<DirectX::CommonStates> m_states;

    //長さ
    float m_lifetime;
    //○秒だけ判定させる
    static constexpr float MAX_LIFETIME = 0.3f;

    //当たり判定用　
    //std::shared_ptr<CollisionAABB> m_collision;

    //std::unique_ptr<ModelCollision> m_collision;
    std::unique_ptr<ModelCollisionOrientedBox> m_collision;

    //DisplayCollision* m_displayCollision = nullptr;

    std::shared_ptr<DisplayCollision> m_displayCollision;

};
