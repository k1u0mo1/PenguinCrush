// AttackP.h
#pragma once

#include "pch.h"

#include "Common/DeviceResources.h"
#include "AttackBase.h"

#include "Common/StepTimer.h"

#include "Camera/PlayerCamera.h"

#include <Collision/DisplayCollision.h>
#include <Collision/ModelCollision.h>


class AttackP:public AttackBase
{
public:
    AttackP(
        DX::DeviceResources* deviceResources,
        const DirectX::SimpleMath::Vector3& playerPos,
        const DirectX::SimpleMath::Vector3& forward,
        std::shared_ptr<DisplayCollision> displayCollision);

   
    void Update(float deltaTime);

    void Render(
        ID3D11DeviceContext* context,
        const DirectX::SimpleMath::Matrix& view,
        const DirectX::SimpleMath::Matrix& proj);

    //生きているか
    bool IsAlive() const;

    bool IsDead() const override { return !IsAlive(); }

    //攻撃を無効化する関数
    virtual void SetDead() override { m_isDead = true; }

    //ノックバックの大きさ
    virtual float GetKnockbackPower() const override { return 15.0f; }

    ModelCollision* GetCollision() const override { return m_collision.get(); }

    DirectX::SimpleMath::Vector3 GetPosition() const override { return m_position; }

    //当たり判定を返す
    //std::shared_ptr<CollisionAABB> GetCollision() const { return m_collision; }

    ModelCollision* GetCollision() { return m_collision.get(); }

    //範囲
    DirectX::BoundingBox GetBoundingBox() const;

    //ダメージ量
    float GetDamage() const override { return 15.0f; }

private:

    DX::DeviceResources* m_deviceResources;

    std::shared_ptr<DirectX::Model> m_attackModel;

    DirectX::SimpleMath::Vector3 m_position;

    DirectX::SimpleMath::Vector3 m_forward;

    std::unique_ptr<DirectX::CommonStates> m_states;

    //長さ
    float m_lifetime;
    //○秒だけ判定させる
    static constexpr float MAX_LIFETIME = 0.1f;

    bool m_isDead = false;

    //当たり判定用　
    //std::shared_ptr<CollisionAABB> m_collision;

    //std::unique_ptr<Ito::ModelCollision> m_collision;
    std::unique_ptr<ModelCollisionOrientedBox> m_collision;

    //Ito::DisplayCollision* m_displayCollision = nullptr;

    std::shared_ptr<DisplayCollision> m_displayCollision;

};
