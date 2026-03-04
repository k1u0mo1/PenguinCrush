// RushP.h
#pragma once

#include "pch.h"

#include "Common/DeviceResources.h"
#include "Common/StepTimer.h"

#include "Camera/PlayerCamera.h"

#include "Collision/DisplayCollision.h"
#include "Collision/ModelCollision.h"
#include "PlayerList/AttackList/AttackBase.h"

class Player;

/// <summary>
/// プレイヤーの突進攻撃を管理するクラス
/// </summary>
class RushP : public AttackBase
{
public:

    /// <summary>
    /// 突進攻撃のインスタンスを生成
    /// プレイヤーの前方に当たり判定を発生
    /// </summary>
    /// <param name="deviceResources">デバイスリソース</param>
    /// <param name="player">突進を行っているプレイヤーのポインタ</param>
    /// <param name="forward">突進する方向のベクトル</param>
    /// <param name="displayCollision">デバッグ表示用のコリジョン</param>
    RushP(
        DX::DeviceResources* deviceResources,
        Player* player,
        const DirectX::SimpleMath::Vector3& forward,
        std::shared_ptr<DisplayCollision> displayCollision);

    /// <summary>
     /// 攻撃の更新処理
     /// </summary>
     /// <param name="dt">前フレームからの経過時間</param>
    void Update(float deltaTime);

    /// <summary>
    /// 攻撃のモデルやエフェクトを描画
    /// </summary>
    /// <param name="context"></param>
    /// <param name="view"></param>
    /// <param name="proj"></param>
    void Render(
        ID3D11DeviceContext* context,
        const DirectX::SimpleMath::Matrix& view,
        const DirectX::SimpleMath::Matrix& proj);

    /// <summary>
    /// 攻撃が終了したか判断
    /// </summary>
    /// <returns></returns>
    bool IsDead() const override;

    /// <summary>
    /// 攻撃を強制的に無効化
    /// </summary>
    void SetDead() override;

    /// <summary>
    /// 敵にヒットした際に与えるノックバックの強さを取得
    /// </summary>
    /// <returns></returns>
    float GetKnockbackPower() const override;

    /// <summary>
    /// 攻撃の当たり判定オブジェクトを取得
    /// </summary>
    /// <returns>当たり判定のポインタ</returns>
    ModelCollision* GetCollision() const override { return m_collision.get(); }

    /// <summary>
    /// 現在のワールド座標を取得
    /// </summary>
    /// <returns></returns>
    DirectX::SimpleMath::Vector3 GetPosition() const override { return m_position; }

    /// <summary>
    /// 当たり判定を返す
    /// </summary>
    /// <returns></returns>
    ModelCollision* GetCollision() { return m_collision.get(); }

    /// <summary>
    /// 攻撃の当たり判定を直接取得
    /// </summary>
    /// <returns></returns>
    DirectX::BoundingBox GetBoundingBox() const;

    /// <summary>
    /// 突進攻撃のダメージ量
    /// </summary>
    /// <returns>{ return 25.0f; }ここでダメージ量を設定</returns>
    float GetDamage() const override { return 25.0f; }
    
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
