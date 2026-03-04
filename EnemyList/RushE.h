// RushE.h
#pragma once

#include "pch.h"

#include "Common/DeviceResources.h"
#include "Common/StepTimer.h"

#include "Camera/PlayerCamera.h"

#include "Collision/DisplayCollision.h"

#include "Collision/ModelCollision.h"


#include "EnemyList/EnemyAttackBase.h"

class BossEnemy;

class RushE : public EnemyAttackBase
{
public:
    //コンストラクタ
    RushE(
        DX::DeviceResources* deviceResources,
        /*const DirectX::SimpleMath::Vector3& playerPos,*/
        BossEnemy* boss,
        const DirectX::SimpleMath::Vector3& forward,
        std::shared_ptr<DisplayCollision> displayCollision);

   //更新
    void Update(float deltaTime) override;

    //描画
    void Render(
        ID3D11DeviceContext* context,
        const DirectX::SimpleMath::Matrix& view,
        const DirectX::SimpleMath::Matrix& proj)override;

    bool IsDead() const override { return m_lifetime >= MAX_LIFETIME; }
   
    void SetDead() override { m_lifetime = MAX_LIFETIME; } 

    //ノックバック力を設定
    float GetKnockbackPower() const override { return 25.0f; } 
    
    ModelCollision* GetCollision() const override { return m_collision.get(); }
    
    DirectX::SimpleMath::Vector3 GetPosition() const override { return m_position; }
    //進行方向
    DirectX::SimpleMath::Vector3 GetForward() const override { return m_forward; }

    //ダメージ量
    float GetDamage() const override { return 30.0f; }

private:

    //範囲
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
