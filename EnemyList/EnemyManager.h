
//EnemyManager　
//敵関連をまとめる用

#pragma once

#include "pch.h"
#include <memory>
#include <vector>

#include "EnemyList/BossEnemy.h"

//影
#include "ShadowRenderer/ShadowRenderer.h"

#include <CommonStates.h>

//攻撃関連--------------------
#include <EnemyList/EnemyAttackList/AttackE.h>
#include <EnemyList/EnemyAttackList/BulletE.h>
#include <EnemyList/EnemyAttackList/RushE.h>
//----------------------------
class Player;
class Particle;

class EnemyManager
{
public:

	EnemyManager();
	~EnemyManager();

    /// <summary>
    /// 敵のマネージャーの初期化
    /// </summary>
    /// <param name="deviceResources">デバイスリソース</param>
    /// <param name="stage">ステージ情報</param>
    /// <param name="displayCollision">敵の攻撃にも共有する表示コリジョン</param>
    void Initialize(
        DX::DeviceResources* deviceResources,
        Stage* stage,
        std::shared_ptr<DisplayCollision> displayCollision);

    /// <summary>
    /// 敵の更新処理と敵の生成管理
    /// </summary>
    /// <param name="dt">前フレームからの経過時間</param>
    /// <param name="player">プレイヤーのポインタ</param>
    /// <param name="particle">エフェクト発生用マネージャー</param>
    void Update(float dt,  Player* player, Particle* particle);

    /// <summary>
    /// 敵と敵の攻撃の描画
    /// </summary>
    /// <param name="context"></param>
    /// <param name="view"></param>
    /// <param name="proj"></param>
    /// <param name="shadowRenderer">影の描画のポインタ</param>
    void Render(
        ID3D11DeviceContext* context,
        const DirectX::SimpleMath::Matrix& view,
        const DirectX::SimpleMath::Matrix& proj,
        ShadowRenderer* shadowRenderer
    );

    /// <summary>
    /// 現在ステージにボスが存在するか判定
    /// </summary>
    /// <returns>ボスが存在する場合は true、未生成の場合は false を返す</returns>
    bool HasBoss() const { return (bool)m_boss; }

    /// <summary>
    /// 現在のボスのオブジェクトを取得
    /// </summary>
    /// <returns>ボスが存在しない場合は nullptr を返す</returns>
    BossEnemy* GetBoss() { return m_boss.get(); }
    
    BossEnemy* GetBossEnemy() const;

    /// <summary>
    /// 現在生きているすべての敵のリストを取得
    /// </summary>
    /// <returns>生きている敵のポインタ配列</returns>
    std::vector<BossEnemy*> GetActiveEnemies()const;

    /// <summary>
    /// ボスの攻撃を実行
    /// </summary>
    /// <param name="dt">前フレームからの経過時間</param>
    /// <param name="playerPos">プレイヤーの現在の座標</param>
    void DoBossAttack(float dt, const DirectX::SimpleMath::Vector3& playerPos);

    /// <summary>
    /// 指定したHPでボスをステージに出現
    /// </summary>
    /// <param name="hp">ボスの初期のHP</param>
    void SpawnBoss(float hp);

private:

    DX::DeviceResources* m_deviceResources = nullptr;
    Stage* m_stage = nullptr;

    // 今回はボス1体だけ
    std::unique_ptr<BossEnemy> m_boss;

    // DisplayCollision（全攻撃で共有）
    std::shared_ptr<DisplayCollision> m_displayCol;

    //弾
    std::shared_ptr<DirectX::Model> m_bulletModel;

    //攻撃のクールタイマー
    float m_attackCoolTimer = 0.0f;

    //攻撃関連
    std::vector<std::unique_ptr<AttackE>> m_attacks;
    std::vector<std::unique_ptr<BulletE>> m_bullets;
    std::vector<std::unique_ptr<RushE>>  m_rushs;

    //ステート
    std::unique_ptr<DirectX::CommonStates> m_states;

};

