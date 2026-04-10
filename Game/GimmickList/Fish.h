// Fish.h
#pragma once

#include "pch.h"

#include "Game/Common/DeviceResources.h"
#include "Game/Common/StepTimer.h"

#include "Game/Camera/PlayerCamera.h"
#include "Game/Collision/DisplayCollision.h"
#include "Game/Collision/ModelCollision.h"

#include "Game/GimmickList/Stage.h"

//影
#include "Game/ShadowRenderer/ShadowRenderer.h"

//傾き
#include "Game/GimmickList/SlideBehavior.h"

/// <summary>
/// ゲーム内に登場する魚オブジェクト
/// プレイヤーの弾薬を回復させる役割
/// </summary>
class Fish 
{
private:

    //------------------------------------------------------
    //生存時間関連
    //------------------------------------------------------

    //魚が消滅するまでの時間
    static constexpr float MAX_LIFETIME = 13.0f;

    //------------------------------------------------------
    //描画＆スケール関連
    //------------------------------------------------------

    //魚モデルのX軸スケール
    static constexpr float FISH_SCALE_X = 0.6f;
    //魚モデルのY軸スケール
    static constexpr float FISH_SCALE_Y = 0.5f;
    //魚モデルのZ軸スケール
    static constexpr float FISH_SCALE_Z = 0.5f;
    //モデルの初期向き補正
    static constexpr float MODEL_ROTATION_OFFSET = 90.0f;

    //------------------------------------------------------
    //当たり判定関連
    //------------------------------------------------------

    //箱型当たり判定のサイズ
    static constexpr float BOUNDING_BOX_SIZE = 1.0f;
    //円型当たり判定の半径
    static constexpr float BOUNDING_SPHERE_RADIUS = 1.5f;

    //------------------------------------------------------
    //アニメーション関連
    //------------------------------------------------------

    //跳ねるアニメーションの速度
    static constexpr float FLOP_SPEED = 20.0f;
    //跳ねる高さの最大値
    static constexpr float FLOP_HEIGHT = 2.0f;

public:

    /// <summary>
    /// コンストラクタ
    /// </summary>
    /// <param name="deviceResources">デバイスリソース</param>
    /// <param name="pos">初期座標</param>
    /// <param name="dir">初期の向き</param>
    /// <param name="displayCollision">表示用の当たり判定オブジェクト</param>
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
    /// <param name="deltaTime">前フレームからの経過時間</param>
    /// <param name="stage">ステージの傾きをもらうポインタ</param>
    void Update(float deltaTime, const Stage* stage);

    /// <summary>
    /// 魚の描画
    /// 跳ねるアニメーション
    /// </summary>
    /// <param name="context">デバイスコンテキスト</param>
    /// <param name="view">ビュー行列</param>
    /// <param name="proj">プロジェクション行列</param>
    /// <param name="shadowRenderer">影を付ける用のポインタ</param>
    /// <param name="stage">足場のポインタ</param>
    void Render(ID3D11DeviceContext* context,
        const DirectX::SimpleMath::Matrix& view,
        const DirectX::SimpleMath::Matrix& proj,
        ShadowRenderer* shadowRenderer,
        const Stage* stage
    );
    
    /// <summary>
    /// 魚が現在生存しているか確認
    /// </summary>
    /// <returns>残りの体力</returns>
    bool IsAlive() const;

    /// <summary>
    /// 弾が当たった際などに呼ばれ、魚を消滅
    /// </summary>
    void BulletKill();

    /// <summary>
    /// 魚に触れた時に得られる弾薬の増加量を取得
    /// </summary>
    /// <returns>弾薬の回復量</returns>
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
    /// <returns>コリジョン</returns>
    ModelCollision* GetCollision() { return m_collision.get(); }
   
    /// <summary>
    /// 魚の当たり判定コリジョン　箱
    /// </summary>
    /// <returns>箱のコリジョン</returns>
    DirectX::BoundingBox GetBoundingBox() const;
    
    /// <summary>
    /// 魚の当たり判定コリジョン　円
    /// </summary>
    /// <returns>円のコリジョン</returns>
    DirectX::BoundingSphere GetBoundingSphere() const { return m_sphere; }

private:

    DirectX::SimpleMath::Vector3 m_position;
    DirectX::SimpleMath::Vector3 m_direction;
    
    float m_lifetime;

    
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

    //滑る挙動を管理するコンポーネント
    SlideBehavior m_sligeBehavior;

};
