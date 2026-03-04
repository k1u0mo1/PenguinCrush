// AttackP.h
#pragma once

#include "pch.h"

#include "Common/DeviceResources.h"
#include "AttackBase.h"

#include "Common/StepTimer.h"

#include "Camera/PlayerCamera.h"

#include <Collision/DisplayCollision.h>
#include <Collision/ModelCollision.h>

/// <summary>
/// プレイヤーの近距離攻撃を管理するクラス
/// </summary>
class AttackP:public AttackBase
{
public:

    /// <summary>
    /// 近距離攻撃のインスタンスを生成
    /// プレイヤーの目の前に当たり判定を配置
    /// </summary>
    /// <param name="deviceResources">デバイスリソース</param>
    /// <param name="playerPos">攻撃を放つプレイヤーの現在の座標</param>
    /// <param name="forward">プレイヤーの向いている方向</param>
    /// <param name="displayCollision">デバッグ表示用のコリジョン管理オブジェクト</param>
    AttackP(
        DX::DeviceResources* deviceResources,
        const DirectX::SimpleMath::Vector3& playerPos,
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
    /// 攻撃の判定がまだ残っているかを返す
    /// </summary>
    /// <returns></returns>
    bool IsAlive() const;

    /// <summary>
    /// 攻撃が終了したか判断
    /// </summary>
    /// <returns>終了している場合はtrue、まだ有効な場合はfalseを返す</returns>
    bool IsDead() const override { return !IsAlive(); }

    /// <summary>
    /// 攻撃を強制的に無効化
    /// </summary>
    virtual void SetDead() override { m_isDead = true; }

    /// <summary>
    /// 敵にヒットした際に与えるノックバックの強さを取得
    /// </summary>
    /// <returns>ノックバック力</returns>
    virtual float GetKnockbackPower() const override { return 15.0f; }

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

    //当たり判定を返す
    //std::shared_ptr<CollisionAABB> GetCollision() const { return m_collision; }

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
    /// 近距離攻撃のダメージ量
    /// </summary>
    /// <returns>{ return 15.0f; }ここでダメージ量を設定</returns>
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
