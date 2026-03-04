// BulletE.h
#pragma once

#include "pch.h"

#include "Common/DeviceResources.h"
#include "Common/StepTimer.h"

#include "Camera/PlayerCamera.h"
#include "Collision/DisplayCollision.h"
#include "Collision/ModelCollision.h"

#include "GimmickList/Stage.h"

#include "EnemyList/EnemyAttackBase.h"

//影
class ShadowRenderer;

class BulletE : public EnemyAttackBase
{
public:
    BulletE(DX::DeviceResources* deviceResources,
        const DirectX::SimpleMath::Vector3& pos,
        const DirectX::SimpleMath::Vector3& dir,
        std::shared_ptr<DisplayCollision> displayCollision,
        std::shared_ptr<DirectX::Model> model,
        float speed = 1.0f);

    

    void Update(float deltaTime) override;
    void Update(float deltaTime, const Stage* stage);

    
    void Render(ID3D11DeviceContext* context,
        const DirectX::SimpleMath::Matrix& view,
        const DirectX::SimpleMath::Matrix& proj
    );

    //弾を消すよう
    bool IsAlive() const;
    
    void BulletKill();

    // コピー禁止
    BulletE(const BulletE&) = delete;
    BulletE& operator=(const BulletE&) = delete;

    // ムーブ許可
    BulletE(BulletE&&) = default;
    BulletE& operator=(BulletE&&) = default;
    
    DirectX::BoundingBox GetBoundingBox() const;
    
    
    bool IsDead() const override { return !m_isAlive || (m_lifetime >= MAX_LIFETIME); }

    void SetDead() override { m_isAlive = false; }

    float GetKnockbackPower() const override { return 10.0f; }

    ModelCollision* GetCollision() const override { return m_collision.get(); }
    
    DirectX::SimpleMath::Vector3 GetPosition() const override { return m_position; }

    DirectX::SimpleMath::Vector3 GetForward() const override { return m_direction; }
   
    //ダメージ量
    float GetDamage()const override { return 10.0f; }

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
