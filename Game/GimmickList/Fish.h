
/**
 * @file   Fish.h
 * @brief  魚オブジェクト
 * @author 國田知睦
 * @date   2026/07/01
 */

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
/// プレイヤーを回復させる役割
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

    //影の大きさ
	static constexpr float SHADOW_SCALE = 1.5f;

    //------------------------------------------------------
    //動き関連
    //------------------------------------------------------
    //揺れ
    static constexpr float FISH_RADIAN = 15.0f;

    //魚の跳ねる強さ
	static constexpr float FISH_JUMP_STRENGTH = 0.5f;
    //跳ねる判定の高さ
	static constexpr float FISH_JUMP_HEIGHT = 1.0f;
    //跳ねる乱数の最大値
    static constexpr int JUMP_PROBABILITY_MAX = 100;
    //跳ねる乱数の最小値
    static constexpr int JUMP_PROBABILITY_MIN = 3;
    //跳ねる力の最小値
    static constexpr float JUMP_FORCE_MIN = 3.0f;
    //跳ねる力の振れ幅
    static constexpr float JUMP_POWER_VARIANCE = 2.0f;
	//跳ねた後にずれるX軸調整値
    static constexpr float JUMP_OFFSET_X_SLIDE = 5.0f;
    //跳ねた後にずれるX軸の倍率
    static constexpr float JUMP_OFFSET_X_MUL = 0.05f;

    //------------------------------------------------------
    //当たり判定関連
    //------------------------------------------------------
    //箱型当たり判定のサイズ
    static constexpr float BOUNDING_BOX_SIZE = 1.5f;
    //円型当たり判定の半径
    static constexpr float BOUNDING_SPHERE_RADIUS = 2.5f;

	//当たり判定の箱の大きさ
    static constexpr float COLLISION_EXTENT_X = 1.0f;
    static constexpr float COLLISION_EXTENT_Y = 1.0f;
    static constexpr float COLLISION_EXTENT_Z = 2.0f;

    //------------------------------------------------------
    //アニメーション関連
    //------------------------------------------------------
    //跳ねるアニメーションの速度
    static constexpr float FLOP_SPEED = 20.0f;
    //跳ねる高さの最大値
    static constexpr float FLOP_HEIGHT = 5.0f;

    //重力
    static constexpr float GRAVITY = -9.8f;

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
    //魚の座標
    DirectX::SimpleMath::Vector3 m_position;
    //魚の向き
    DirectX::SimpleMath::Vector3 m_direction;
    //生存時間
    float m_lifetime;
    //デバイスリソース
    DX::DeviceResources* m_deviceResources;

    //射影行列
    DirectX::SimpleMath::Matrix m_proj;

    std::unique_ptr<DirectX::CommonStates> m_states;
    std::shared_ptr<DirectX::Model> m_model;

    //当たり判定用コリジョン
    std::unique_ptr<ModelCollisionOrientedBox> m_collision;
    std::shared_ptr<DisplayCollision> m_displayCollision;

    //足元から原点の高さ
    float m_fishHeightOffset;
    //重力
    DirectX::SimpleMath::Vector3 m_velocity = DirectX::SimpleMath::Vector3::Zero;
    
    
    //当たり判定Sphere
    DirectX::BoundingSphere m_sphere;

    //滑る挙動を管理するコンポーネント
    SlideBehavior m_sligeBehavior;

};
