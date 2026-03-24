// BulletP.h
#pragma once

#include "pch.h"
#include "PlayerList/AttackList/AttackBase.h" 
#include "Common/DeviceResources.h"
#include "Collision/DisplayCollision.h"
#include "Collision/ModelCollision.h"
#include "GimmickList/Stage.h" 

#include <Model.h>
#include <SimpleMath.h>
#include <memory>

/// <summary>
/// プレイヤーが発射する遠距離攻撃
/// </summary>
class BulletP : public AttackBase
{
private:

    //------------------------------------------------------
    //定数関連
    //------------------------------------------------------

    //攻撃したときのダメージ量
    static constexpr float BULLET_DAMAGE = 10.0f;
    //敵を弾き飛ばすノックバック力
    static constexpr float KNOCKBACK_POWER = 10.0f;
    //攻撃判定が消えるまでの時間
    static constexpr float MAX_LIFETIME = 3.0f;
    //当たり判定の大きさ
    static constexpr float COLLISION_SIZE = 0.25f;

public:

    /// <summary>
    /// コンストラクタ
    /// </summary>
    /// <param name="pos">発生座標</param>
    /// <param name="dir">進行方向</param>
    /// <param name="model">描画用モデル</param>
    /// <param name="states">描画ステート</param>
    /// <param name="stage">地形判定用のステージポインタ</param>
    /// <param name="displayCollision">デバッグ表示用コリジョン</param>
    /// <param name="speed">弾の速度</param>
    /// <param name="damage">弾のダメージ</param>
    BulletP(
        const DirectX::SimpleMath::Vector3& pos,
        const DirectX::SimpleMath::Vector3& dir,
        std::shared_ptr<DirectX::Model> model,
        DirectX::CommonStates* states,
        const Stage* stage,
        std::shared_ptr<DisplayCollision> displayCollision,
        float speed = 25.0f
    );

    /// <summary>
    /// 攻撃の更新処理
    /// </summary>
    /// <param name="dt">前フレームからの経過時間</param>
    void Update(float dt) override;

    /// <summary>
    /// 攻撃のモデルやエフェクトを描画
    /// </summary>
    /// <param name="context"></param>
    /// <param name="view"></param>
    /// <param name="proj"></param>
    void Render(
        ID3D11DeviceContext* context,
        const DirectX::SimpleMath::Matrix& view,
        const DirectX::SimpleMath::Matrix& proj
    ) override;

    /// <summary>
    /// 攻撃が終了したか判断
    /// </summary>
    /// <returns>終了している場合はtrue、まだ有効な場合はfalseを返す</returns>
    bool IsDead() const override { return m_isDead; }
    
    /// <summary>
    /// 攻撃を強制的に無効化
    /// </summary>
    void SetDead() override { m_isDead = true; }
    
    /// <summary>
    /// 敵にヒットした際に与えるノックバックの強さを取得
    /// </summary>
    /// <returns>ノックバック力</returns>
    float GetKnockbackPower() const override { return KNOCKBACK_POWER; }
    
    /// <summary>
    /// 攻撃の当たり判定オブジェクトを取得
    /// </summary>
    /// <returns>当たり判定のポインタ</returns>
    ModelCollision* GetCollision() const override { return m_collision.get(); }
    
    /// <summary>
    /// 現在のワールド座標を取得
    /// </summary>
    /// <returns></returns>
    DirectX::SimpleMath::Vector3 GetPosition() const override { return m_pos; }
    
    /// <summary>
    /// 遠距離攻撃のダメージ量
    /// </summary>
    /// <returns>ダメージ量</returns>
    float GetDamage() const override { return BULLET_DAMAGE; }
    

private:

    DirectX::SimpleMath::Vector3 m_pos;
    
    DirectX::SimpleMath::Vector3 m_dir;

    //弾の速さ
    float m_speed;

    float m_lifetime;

    bool m_isDead;

    
    std::shared_ptr<DirectX::Model> m_model;
    
    DirectX::CommonStates* m_states;
    
    const Stage* m_stage;
    
    std::shared_ptr<DisplayCollision> m_displayCollision;
    
    std::unique_ptr<ModelCollisionOrientedBox> m_collision;
};