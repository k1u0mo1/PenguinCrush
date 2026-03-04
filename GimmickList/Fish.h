// Fish.h
#pragma once

#include "pch.h"

#include "Common/DeviceResources.h"
#include "Common/StepTimer.h"

#include "Camera/PlayerCamera.h"
#include "Collision/DisplayCollision.h"
#include "Collision/ModelCollision.h"

#include "GimmickList/Stage.h"

//影
#include "ShadowRenderer/ShadowRenderer.h"

/// <summary>
/// ゲーム内に登場する魚オブジェクト
/// プレイヤーの弾薬を回復させる役割
/// </summary>
class Fish 
{
public:

    //コンストラクタ
    Fish(DX::DeviceResources* deviceResources,
        const DirectX::SimpleMath::Vector3& pos,
        const DirectX::SimpleMath::Vector3& dir,
        std::shared_ptr<DisplayCollision> displayCollision
        );

    /// <summary>
    /// 魚の初期化
    /// </summary>
    void Initialize();

    /// <summary>
    /// 魚の更新
    /// </summary>
    /// <param name="deltaTime"></param>
    /// <param name="stage">ステージの傾きをもらうポインタ</param>
    void Update(float deltaTime, const Stage* stage);

    /// <summary>
    /// 魚の描画
    /// 跳ねるアニメーション
    /// </summary>
    /// <param name="context"></param>
    /// <param name="view"></param>
    /// <param name="proj"></param>
    /// <param name="shadowRenderer">影を付ける用のポインタ</param>
    /// <param name="stage"></param>
    void Render(ID3D11DeviceContext* context,
        const DirectX::SimpleMath::Matrix& view,
        const DirectX::SimpleMath::Matrix& proj,
        ShadowRenderer* shadowRenderer,
        const Stage* stage
    );
    
    /// <summary>
    /// 魚が現在生存しているか確認
    /// </summary>
    /// <returns></returns>
    bool IsAlive() const;

    /// <summary>
    /// 弾が当たった際などに呼ばれ、魚を消滅
    /// </summary>
    void BulletKill();

    /// <summary>
    /// 魚に触れた時に得られる弾薬の増加量を取得
    /// </summary>
    /// <returns>弾薬の回復量　m_ammoValue</returns>
    int GetAmmoValue() const { return m_ammoValue; }

    // コピー禁止
    Fish(const Fish&) = delete;
    Fish& operator=(const Fish&) = delete;

    // ムーブ許可
    Fish(Fish&&) = default;
    Fish& operator=(Fish&&) = default;
    
    /// <summary>
    /// 魚の現在位置を取得
    /// </summary>
    /// <returns>m_position</returns>
    DirectX::SimpleMath::Vector3 GetPosition() const;
   
    /// <summary>
    /// 当たり判定用のコリジョンオブジェクトを取得
    /// </summary>
    /// <returns>m_collision</returns>
    ModelCollision* GetCollision() { return m_collision.get(); }
   
    /// <summary>
    /// 魚の当たり判定コリジョン　箱
    /// </summary>
    /// <returns>box</returns>
    DirectX::BoundingBox GetBoundingBox() const;
    
    /// <summary>
    /// 魚の当たり判定コリジョン　円
    /// </summary>
    /// <returns>m_sphere</returns>
    DirectX::BoundingSphere GetBoundingSphere() const { return m_sphere; }

private:

    DirectX::SimpleMath::Vector3 m_position;
    DirectX::SimpleMath::Vector3 m_direction;
    
    float m_lifetime;

    static constexpr float MAX_LIFETIME = 13.0f;

    DX::DeviceResources* m_deviceResources;

    //  射影行列
    DirectX::SimpleMath::Matrix m_proj;

    std::unique_ptr<DirectX::CommonStates> m_states;
    //std::unique_ptr<DirectX::Model> m_bulletModel;
    std::shared_ptr<DirectX::Model> m_model;

    std::unique_ptr<ModelCollisionOrientedBox> m_collision;

    //※参照を持たせる
    //DisplayCollision* m_displayCollision = nullptr;
    std::shared_ptr<DisplayCollision> m_displayCollision;

    //足元から原点の高さ
    float m_fishHeightOffset;

    //重力
    DirectX::SimpleMath::Vector3 m_velocity = DirectX::SimpleMath::Vector3::Zero;
    //重力加速度
    float m_gravity = -9.8f;

    //魚を回収したときの増える弾数
    int m_ammoValue = 1;

    //当たり判定Sphere
    DirectX::BoundingSphere m_sphere;

};
