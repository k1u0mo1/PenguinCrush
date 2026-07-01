
/**
 * @file   RushP.h
 * @brief  プレイヤーの突進攻撃を管理するクラスの実装
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
#include "Game/PlayerList/AttackList/AttackBase.h"

class Player;

/// <summary>
/// プレイヤーの突進攻撃を管理するクラス
/// </summary>
class RushP : public AttackBase
{
private:

    //攻撃したときのダメージ量
    static constexpr float ATTACK_DAMAGE = 25.0f;
    //敵を吹き飛ばすノックバック力
    static constexpr float KNOCKBACK_POWER = 30.0f;
    //攻撃判定が消えるまでの時間
    static constexpr float MAX_LIFETIME = 0.5f;
    //突進のスピード
    static constexpr float RUSH_SPEED = 25.0f;

    //サイズ
    static constexpr float SCALE_SIZE = 1.0f;
    //突進方向
    static constexpr DirectX::SimpleMath::Vector3 FORWARD =
        DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.1f);

    //当たり判定のサイズ
    static constexpr DirectX::SimpleMath::Vector3 COLLISION_SIZE =
        DirectX::SimpleMath::Vector3(1.0f, 1.5f, 1.0f);

    //デバックの当たり判定の線の太さ
    static constexpr float DEBUG_COLLISION_LINE_THICKNESS = 0.5f;
    //デフォルトの箱のサイズ
    static constexpr DirectX::SimpleMath::Vector3 DEFAULT_BOX_SIZE =
        DirectX::SimpleMath::Vector3(0.3f, 0.3f, 0.3f);

    //ゼロ除算を防ぐ小さい値
    static constexpr float EPSILON = 0.001f;

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
    /// <param name="context">デバイスリソース</param>
    /// <param name="view">ビュー行列</param>
    /// <param name="proj">射影行列</param>
    void Render(
        ID3D11DeviceContext* context,
        const DirectX::SimpleMath::Matrix& view,
        const DirectX::SimpleMath::Matrix& proj);

    /// <summary>
    /// 攻撃が終了したか判断
    /// </summary>
    /// <returns>trueなら攻撃終了</returns>
    bool IsDead() const override;

    /// <summary>
    /// 攻撃を強制的に無効化
    /// </summary>
    void SetDead() override;

    /// <summary>
    /// 敵にヒットした際に与えるノックバックの強さを取得
    /// </summary>
    /// <returns>ノックバックの強さ</returns>
    float GetKnockbackPower() const { return KNOCKBACK_POWER; }

    /// <summary>
    /// 攻撃の当たり判定オブジェクトを取得
    /// </summary>
    /// <returns>当たり判定のポインタ</returns>
    ModelCollision* GetCollision() const override { return m_collision.get(); }

    /// <summary>
    /// 現在のワールド座標を取得
    /// </summary>
    /// <returns>現在のワールド座標</returns>
    DirectX::SimpleMath::Vector3 GetPosition() const override { return m_position; }

    /// <summary>
    /// 当たり判定を返す
    /// </summary>
    /// <returns>当たり判定のポインタ</returns>
    ModelCollision* GetCollision() { return m_collision.get(); }

    /// <summary>
    /// 攻撃の当たり判定を直接取得
    /// </summary>
    /// <returns>攻撃判定のボックス</returns>
    DirectX::BoundingBox GetBoundingBox() const;

    /// <summary>
    /// 突進攻撃のダメージ量
    /// </summary>
    /// <returns>ダメージ量</returns>
    float GetDamage() const override { return ATTACK_DAMAGE; }
    
    /// <summary>
    /// 攻撃を撃ったプレイヤーを取得
    /// 突進後のデメリットを作るため用
    /// </summary>
    /// <returns>m_player</returns>
    Player* GetPlayer() const { return m_player; }

private:
    //デバイスリソース
    DX::DeviceResources* m_deviceResources;
    //モデル
    std::shared_ptr<DirectX::Model> m_rushModel;
    std::shared_ptr<DirectX::Model> m_model;

    //現在の座標
    DirectX::SimpleMath::Vector3 m_position;
    //方向
    DirectX::SimpleMath::Vector3 m_forward;

    std::unique_ptr<DirectX::CommonStates> m_states;

    //長さ
    float m_lifetime;
    
    //当たり判定
    std::unique_ptr<ModelCollisionOrientedBox> m_collision;
    std::shared_ptr<DisplayCollision> m_displayCollision;
 
    bool m_isDead = false;

    Player* m_player;
};
