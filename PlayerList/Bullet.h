
//Bullet.h
//弾のクラス

#pragma once
#include "pch.h"
#include "AttackBase.h"

#include "Common/DeviceResources.h"
#include "Common/StepTimer.h"
#include "Collision/DisplayCollision.h"
#include "Collision/ModelCollision.h"
#include "GimmickList/Stage.h"

#include <memory>

class Bullet : public AttackBase
{
public:

    //コンストラクタ
    Bullet(
        DX::DeviceResources* deviceResources,
        const DirectX::SimpleMath::Vector3& pos,
        const DirectX::SimpleMath::Vector3& dir,
        const Stage* stage,
        std::shared_ptr<DisplayCollision> displayCollision,
        float speed = 1.0f,
        float damage = 10.0f
    );

    // AttackBase override ----
    //更新
    void Update(float dt) override;
    //描画
    void Render(
        ID3D11DeviceContext* context,
        const DirectX::SimpleMath::Matrix& view,
        const DirectX::SimpleMath::Matrix& proj
    ) override;

    //やられたか？
    bool IsDead() const override;

    DirectX::SimpleMath::Vector3 GetPosition() const override
    {
        return m_position;
    }
    // -----------------------------

    float GetDamage() const { return m_damage; }

    DirectX::BoundingBox GetBoundingBox() const;

    void BulletKill();

private:

    DX::DeviceResources* m_deviceResources;
    const Stage* m_stage;

    DirectX::SimpleMath::Vector3 m_position;
    DirectX::SimpleMath::Vector3 m_direction;

    float m_speed;
    float m_lifetime;
    float m_damage;

    bool m_isAlive;
    static constexpr float MAX_LIFETIME = 3.0f;

    std::shared_ptr<DirectX::Model> m_bulletModel;
    std::unique_ptr<ModelCollisionOrientedBox> m_collision;

    std::shared_ptr<DisplayCollision> m_displayCollision;
};

