
/**
 * @file   EnemyManager.h
 * @brief  敵関連をまとめる管理を行うクラス
 * @author 國田知睦
 * @date   2026/07/01
 */

#pragma once
#include "pch.h"
#include <memory>
#include <vector>
//影
#include "Game/ShadowRenderer/ShadowRenderer.h"
#include <CommonStates.h>
//攻撃関連--------------------
#include <Game/EnemyList/EnemyAttackList/AttackE.h>
#include <Game/EnemyList/EnemyAttackList/RushE.h>
//----------------------------
#include "Game/EnemyList/EnemyBaseParameter.h"

class BossEnemy;
class EnemyBase;
class Player;
class Particle;
class Stage;

class EnemyManager
{
private:

    //出現座標
    static constexpr float BOSS_SPAWN_POS_X = 10.0f;
    static constexpr float BOSS_SPAWN_POS_Y = 50.0f;
    static constexpr float BOSS_SPAWN_POS_Z = 10.0f;
    
    //------------------------------------------------------
    //攻撃＆ステート関連
    //------------------------------------------------------

    //突進攻撃のクールタイム
    static constexpr float RUSH_COOLDOWN = 7.0f;
    //近距離攻撃のクールタイム
    static constexpr float ATTACK_COOLDOWN = 1.5f;
    //突進ステートの持続の時間
    static constexpr float RUSH_STATE_DURATION = 1.0f;
    //近距離ステートの持続の時間
    static constexpr float ATTACK_STATE_DURATION = 1.0f;
    //近距離攻撃と突進攻撃の切り替え　
    static constexpr float ATTACK_SWITCH_DISTANCE = 10.0f;
    //突進攻撃と遠距離攻撃の切り替え　
    static constexpr float DASH_SWITCH_DISTANCE = 30.0f;

    //ダッシュ時の高さ
    static constexpr float DASH_HEIGHT = 1.5f;

    //エフェクトの生成数
    static constexpr int HIT_EFFECT_COUNT = 100;
    //エフェクトのサイズ
    static constexpr float HIT_EFFECT_SIZE = 0.5f;
    //突進エフェクト生成数
    static constexpr int DASH_EFFECT_COUNT = 1;
    //突進エフェクトサイズ
    static constexpr float DASH_EFFECT_SIZE = 0.3f;
    //攻撃エフェクト生成時の高さ
    static constexpr float HIT_EFFECT_HEIGHT_OFFSET = 2.0f;
    
    //------------------------------------------------------
    //その他のシステム関連
    //------------------------------------------------------
    //影の大きさ
    static constexpr float SHADOW_SCALE = 3.0f;

    //微小値
    static constexpr float VECTOR_EPSILON = 0.0001f;

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
    bool HasBoss() const;

    /// <summary>
    /// 現在のボスのオブジェクトを取得
    /// </summary>
    /// <returns>ボスが存在する場合はポインタ、存在しない場合は nullptr を返す</returns>
    BossEnemy* GetBoss()const;
    
    
    /// <summary>
    /// 現在のボスのオブジェクトを取得
    /// </summary>
    /// <returns>ボスが存在する場合はポインタ、存在しない場合は nullptr を返す</returns>
    BossEnemy* GetBossEnemy() const;

    /// <summary>
    /// 現在生きているすべての敵のリストを取得
    /// </summary>
    /// <returns>生きている敵のポインタ配列</returns>
    std::vector<EnemyBase*> GetActiveEnemies()const;

    /// <summary>
    /// ボスの攻撃を実行
    /// </summary>
    /// <param name="dt">前フレームからの経過時間</param>
    /// <param name="playerPos">プレイヤーの現在の座標</param>
    void DoBossAttack(
        float deltaTime, 
        const DirectX::SimpleMath::Vector3& playerPos);

    /// <summary>
    /// 指定したHPでボスをステージに出現
    /// </summary>
    void SpawnBoss();
	
	/// <summary>
	/// 指定した位置に通常の敵をステージに出現
	/// </summary>
	/// <param name="position">出現位置</param>
	/// <param name="param">敵データ（個性）</param>
	void SpawnNormalEnemy(
        const DirectX::SimpleMath::Vector3& position,
        const EnemyBaseParameter& param=EnemyData::NormalEnemy);

public:

    /// <summary>
	/// 敵の近距離攻撃を生成
    /// </summary>
    /// <param name="enemy">攻撃を生成する敵のポインタ</param>
    /// <param name="forward">攻撃の前方ベクトル</param>
    void RequestAttackE(
        EnemyBase* enemy,
        const DirectX::SimpleMath::Vector3& forward);

	/// <summary>
	/// 敵の突進攻撃を生成
	/// </summary>
	/// <param name="enemy">攻撃を生成する敵のポインタ</param>
	/// <param name="forward">攻撃の前方ベクトル</param>
	void RequestRushE(
        EnemyBase* enemy,
        const DirectX::SimpleMath::Vector3& forward);

private:

    /// <summary>
    /// 敵同士の重なりを防ぐ
    /// </summary>
    void ResolveEnemyCollisions();

private:

    DX::DeviceResources* m_deviceResources = nullptr;
    Stage* m_stage = nullptr;

    //敵のリスト
	std::vector<std::unique_ptr<EnemyBase>> m_enemies;
    // DisplayCollision（全攻撃で共有）
    std::shared_ptr<DisplayCollision> m_displayCol;

    //攻撃のクールタイマー
    float m_attackCoolTimer = 0.0f;

    //攻撃関連
    std::vector<std::unique_ptr<AttackE>> m_attacks;
    std::vector<std::unique_ptr<RushE>>  m_rush;
    //ステート
    std::unique_ptr<DirectX::CommonStates> m_states;
};

