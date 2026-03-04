// RushP.h
#pragma once

#include "pch.h"

#include "Common/DeviceResources.h"
#include "Common/StepTimer.h"

#include "Camera/PlayerCamera.h"

#include "Collision/DisplayCollision.h"
#include "Collision/ModelCollision.h"
#include "PlayerList/AttackBase.h"

class Player;

class RushP : public AttackBase
{
public:
    RushP(
        DX::DeviceResources* deviceResources,
        Player* player,
        const DirectX::SimpleMath::Vector3& forward,
        std::shared_ptr<DisplayCollision> displayCollision);

   
    void Update(float deltaTime);

    void Render(
        ID3D11DeviceContext* context,
        const DirectX::SimpleMath::Matrix& view,
        const DirectX::SimpleMath::Matrix& proj);


    bool IsDead() const override;

    //攻撃を無効化する関数
    void SetDead() override;

    //ノックバックの大きさ
    float GetKnockbackPower() const override;

    ModelCollision* GetCollision() const override { return m_collision.get(); }

    DirectX::SimpleMath::Vector3 GetPosition() const override { return m_position; }


    
    ModelCollision* GetCollision() { return m_collision.get(); }

    //範囲
    DirectX::BoundingBox GetBoundingBox() const;

    //ダメージ量
    float GetDamage() const override { return 30.0f; }

    
private:

    DX::DeviceResources* m_deviceResources;

    std::shared_ptr<DirectX::Model> m_rushModel;

    DirectX::SimpleMath::Vector3 m_position;

    DirectX::SimpleMath::Vector3 m_forward;

    std::unique_ptr<DirectX::CommonStates> m_states;

    std::shared_ptr<DirectX::Model> m_model;

    //長さ
    float m_lifetime;
    //○秒だけ判定させる
    static constexpr float MAX_LIFETIME = 0.5f;

    std::unique_ptr<ModelCollisionOrientedBox> m_collision;

    std::shared_ptr<DisplayCollision> m_displayCollision;

    float m_knockbackPower = 30.0f; 
    bool m_isDead = false;

    Player* m_player;

};
