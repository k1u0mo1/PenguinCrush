
//EnemyManager　
//敵関連をまとめる用

#pragma once

#include "pch.h"
#include <memory>
#include <vector>

#include "Game/EnemyList/BossEnemy.h"

//影
#include "Game/ShadowRenderer/ShadowRenderer.h"

#include <CommonStates.h>

//攻撃関連--------------------
#include <Game/EnemyList/EnemyAttackList/AttackE.h>
#include <Game/EnemyList/EnemyAttackList/BulletE.h>
#include <Game/EnemyList/EnemyAttackList/RushE.h>
//----------------------------
class Player;
class Particle;

class EnemyManager
{
private:

    //------------------------------------------------------
    //攻撃＆ステート関連
    //------------------------------------------------------

    //突進攻撃のクールタイム
    static constexpr float RUSH_COOLDOWN = 7.0f;
    //近距離攻撃のクールタイム
    static constexpr float ATTACK_COOLDOWN = 1.5f;
    //遠距離攻撃のクールタイム
    static constexpr float SHOOT_COOLDOWN = 5.2f;
    //突進ステートの持続の時間
    static constexpr float RUSH_STATE_DURATION = 1.0f;
    //近距離ステートの持続の時間
    static constexpr float ATTACK_STATE_DURATION = 1.0f;
    //遠距離ステートの持続の時間
    static constexpr float SHOOT_STATE_DURATION = 1.0f;

    //近距離攻撃と突進攻撃の切り替え　
    static constexpr float ATTACK_SWITCH_DISTANCE = 10.0f;
    //突進攻撃と遠距離攻撃の切り替え　
    static constexpr float DASH_SWITCH_DISTANCE = 30.0f;

    //------------------------------------------------------
    //弾の生成位置関連
    //------------------------------------------------------

    //前方への生成距離
    static constexpr float BULLET_SPAWN_DISTANCE = 5.0f;
    //生成位置の高さ
    static constexpr float BULLET_SPAWN_HEIGHT = 1.5f;

    //------------------------------------------------------
    //その他のシステム関連
    //------------------------------------------------------
    
    //影の大きさ
    static constexpr float SHADOW_SCALE = 3.0f;

public:

	/// <summary>
	/// コンストラクタ
	/// </summary>
	EnemyManager();

	/// <summary>
	/// デストラクタ
	/// </summary>
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
    /// <param name="context">デバイスコンテキスト</param>
    /// <param name="view">ビュー行列</param>
    /// <param name="proj">プロジェクション行列</param>
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
    
    /// <summary>
    /// 現在のボスのオブジェクトを取得
    /// </summary>
    /// <returns>ボスが存在しない場合は nullptr を返す</returns>
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
    void DoBossAttack(float deltaTime, const DirectX::SimpleMath::Vector3& playerPos);

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

