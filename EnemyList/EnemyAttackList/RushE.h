// RushE.h
#pragma once

#include "pch.h"

#include "Common/DeviceResources.h"
#include "Common/StepTimer.h"

#include "Camera/PlayerCamera.h"

#include "Collision/DisplayCollision.h"

#include "Collision/ModelCollision.h"

#include "EnemyList/EnemyAttackList/EnemyAttackBase.h"

class BossEnemy;

class RushE : public EnemyAttackBase
{
public:

    /// <summary>
    /// 敵の突進攻撃を生成
    /// </summary>
    /// <param name="deviceResources">デバイスリソース</param>
    /// <param name="boss">突進させるボスのポインタ</param>
    /// <param name="forward">突進する方向ベクトル</param>
    /// <param name="displayCollision">デバッグ描画用</param>
    RushE(
        DX::DeviceResources* deviceResources,
        /*const DirectX::SimpleMath::Vector3& playerPos,*/
        BossEnemy* boss,
        const DirectX::SimpleMath::Vector3& forward,
        std::shared_ptr<DisplayCollision> displayCollision);

    /// <summary>
    /// ボスの向きと座標を更新、突進させる
    /// </summary>
    /// <param name="deltaTime">前フレームからの経過時間</param>
    void Update(float deltaTime) override;

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
    float GetKnockbackPower() const override { return 25.0f; } 
    
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
    /// 突進攻撃のダメージ量を取得
    /// </summary>
    /// <returns>ダメージ量　{ return 30.0f; }</returns>
    float GetDamage() const override { return 30.0f; }

private:

    /// <summary>
    /// 当たり判定の範囲
    /// </summary>
    /// <returns>box</returns>
    DirectX::BoundingBox GetBoundingBox() const;

private:

    DX::DeviceResources* m_deviceResources;

    std::shared_ptr<DirectX::Model> m_rushModel;

    DirectX::SimpleMath::Vector3 m_position;

    DirectX::SimpleMath::Vector3 m_forward;

    std::unique_ptr<DirectX::CommonStates> m_states;

    //長さ
    float m_lifetime;
    //○秒だけ判定させる
    static constexpr float MAX_LIFETIME = 1.5f;

    
    std::unique_ptr<ModelCollisionOrientedBox> m_collision;

    std::shared_ptr<DisplayCollision> m_displayCollision;

    //ボスへのポインタ
    BossEnemy* m_boss;

};
