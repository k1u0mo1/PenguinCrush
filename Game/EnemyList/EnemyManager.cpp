
/**
 * @file   EnemyManager.cpp
 * @brief  敵関連をまとめる管理を行うクラス
 * @author 國田知睦
 * @date   2026/07/01
 */

#include "pch.h"
#include <algorithm>
#include <Model.h>   
#include <Effects.h>
#include "EnemyManager.h"
#include "Game/PlayerList/Player.h"
#include "Game/Effects/Particle.h"
#include <Game/SoundList/AudioManager.h>
#include "Game/EnemyList/BossEnemy.h"
#include "EnemyBase.h"
#include "Game/GimmickList/Stage.h"
#include "NormalEnemy.h"

//----------------------------------------------------------
// コンストラクタ
//----------------------------------------------------------

EnemyManager::EnemyManager()
{
}

//----------------------------------------------------------
// デストラクタ
//----------------------------------------------------------

EnemyManager::~EnemyManager()
{
}

//----------------------------------------------------------
// 敵のマネージャーの初期化
//----------------------------------------------------------

void EnemyManager::Initialize(
	DX::DeviceResources* deviceResources,
	Stage* stage,
    std::shared_ptr<DisplayCollision> displayCollision)
{
    //デバイスリソースの取得
    m_deviceResources = deviceResources;
    m_stage = stage;

    m_displayCol = displayCollision;

    //ステートの作成
    m_states = std::make_unique<DirectX::CommonStates>(deviceResources->GetD3DDevice());

    DirectX::EffectFactory fx(deviceResources->GetD3DDevice());
}

//----------------------------------------------------------
// 敵の更新処理と敵の生成管理
//----------------------------------------------------------

void EnemyManager::Update(
    float dt,  Player* player, Particle* particle)
{
	//敵の更新
    for (auto& enemy : m_enemies)
    {
        enemy->Update(
            dt,
            player->GetPosition(),
            m_stage,
            particle,
            this
		);
    }

    //敵の重なり防止
    ResolveEnemyCollisions();
    //--------------------------------
    //攻撃の更新 
    //--------------------------------
    for (size_t i = 0; i < m_attacks.size(); i++)
    {
        m_attacks[i]->Update(dt);
    }
    
    for (size_t i = 0; i < m_rush.size(); i++)
    {
        m_rush[i]->Update(dt);

        BossEnemy* boss = GetBossEnemy();
        // 突進エフェクト
        if (particle && boss)
        {
            // 現在のボスの位置を取得
            DirectX::SimpleMath::Vector3 pos = boss->GetPosition();
            
            pos.y += DASH_HEIGHT;
           
            //エフェクト生成
            particle->Spawn(
                Particle::Type::Dash,
                pos,
                DASH_EFFECT_COUNT,
                DASH_EFFECT_SIZE);
        }
    }
    
    //-----------------------------------------------
    // 敵の攻撃とプレイヤーの衝突判定＋ダメージ処理
    //-----------------------------------------------
    if (player)
    {
        //Attackの衝突判定
        for (auto& atk : m_attacks)
        {
            //攻撃コリジョンとプレイヤーのコリジョンの衝突判定
            if (atk->GetCollision()->Intersects(player->GetCollision()))
            {
                //プレイヤーにノックバックとダメージ
                player->ApplyKnockback(atk->GetForward(), atk->GetKnockbackPower());

                if (particle)
                {
                    //プレイヤーの位置を取得
                    DirectX::SimpleMath::Vector3 hitPos = player->GetPosition();
                    //高さを少し上げる
                    hitPos.y += HIT_EFFECT_HEIGHT_OFFSET;
                    //エフェクト生成
                    particle->Spawn(
                        Particle::Type::Explosion,
                        hitPos,
                        HIT_EFFECT_COUNT,
                        HIT_EFFECT_SIZE);
                }

                //効果音
                AudioManager::GetInstance()->Play("AttackE");
                //ダメージ適用
                player->TakeDamage(atk->GetDamage());
                //攻撃が当たったら削除
                atk->SetDead();
            }
        }

        //Rushの衝突判定
        for (auto& rush : m_rush)
        {
            //攻撃コリジョンとプレイヤーのコリジョンの衝突判定
            if (rush->GetCollision()->Intersects(player->GetCollision()))
            {
                //プレイヤーにノックバックとダメージ
                player->ApplyKnockback(rush->GetForward(), rush->GetKnockbackPower());
                
                if (particle)
                {
                    // プレイヤーの位置を取得
                    DirectX::SimpleMath::Vector3 hitPos = player->GetPosition();

                    //高さを少し上げる
                    hitPos.y += HIT_EFFECT_HEIGHT_OFFSET;

                    // エフェクト生成
                    particle->Spawn(
                        Particle::Type::Explosion,
                        hitPos,
                        HIT_EFFECT_COUNT,
                        HIT_EFFECT_SIZE);
                }

                //効果音
                AudioManager::GetInstance()->Play("DashE");
                //ダメージ適用
                player->TakeDamage(rush->GetDamage());
                //攻撃が当たったら削除
                rush->SetDead();
            }
        }
    }
    
    //--------------------------------
    // 生存判定：死んだものを削除
    //--------------------------------
    //やられた敵
    m_enemies.erase(
        std::remove_if(m_enemies.begin(), m_enemies.end(),
            [](std::unique_ptr<EnemyBase>& e) { return e->IsDead(); }),
		m_enemies.end());
    //やられた攻撃
    m_attacks.erase(
        std::remove_if(m_attacks.begin(), m_attacks.end(),
            [](std::unique_ptr<AttackE>& a) { return a->IsDead(); }),
        m_attacks.end());
    //やられた突進
    m_rush.erase(
        std::remove_if(m_rush.begin(), m_rush.end(),
            [](std::unique_ptr<RushE>& r) { return r->IsDead(); }),
        m_rush.end());
}

//----------------------------------------------------------
// 敵と敵の攻撃の描画
//----------------------------------------------------------

void EnemyManager::Render(
    ID3D11DeviceContext* context,
    const DirectX::SimpleMath::Matrix& view,
    const DirectX::SimpleMath::Matrix& proj,
    ShadowRenderer* shadowRenderer
)
{
	//敵の描画
    for(auto& enemy : m_enemies)
    {
        enemy->Render(context, view, proj);
	}
    
    BossEnemy* boss = GetBossEnemy();
    //敵の影の描画
    if (boss && shadowRenderer)
    {
        //ボスの現在位置を取得
        DirectX::SimpleMath::Vector3 shadowPos = boss->GetPosition();

        //ステージの傾きを取得
        float rotX = m_stage->GetRotateX();
        float rotZ = m_stage->GetRotateZ();

        //影を描画
        shadowRenderer->Render(
            context,
            m_states.get(),
            view,
            proj,
            shadowPos,
            SHADOW_SCALE,
            rotX,
            rotZ
        );
    }

    //各攻撃の描画
    for (auto& atk : m_attacks)
    {
        atk->Render(context, view, proj);
    }
    
    for (auto& r : m_rush)
    {
        r->Render(context, view, proj);
    }
}

//----------------------------------------------------------
// 敵の攻撃の描画
//----------------------------------------------------------

bool EnemyManager::HasBoss() const
{
    return GetBoss() != nullptr;
}

//----------------------------------------------------------
// 現在生きているすべての敵のリストを取得
//----------------------------------------------------------

std::vector<EnemyBase*> EnemyManager::GetActiveEnemies()const
{
    std::vector<EnemyBase*> enemies;
    //生きている敵をリストに追加
    for (auto& enemy : m_enemies)
    {
        if (!enemy->IsDead())
        {
            enemies.push_back(enemy.get());
        }
    }
    return enemies;
}

//----------------------------------------------------------
// ボスの攻撃を実行
//----------------------------------------------------------

void EnemyManager::DoBossAttack(
    float dt, 
    const DirectX::SimpleMath::Vector3& playerPos
    )
{
    BossEnemy* boss = GetBossEnemy();

    if (!boss) return;

    m_attackCoolTimer -= dt;

    //クールタイマ
    if (m_attackCoolTimer > 0)
    {
        return;
    }
    //敵の座標をもらう
    auto pos = boss->GetPosition();
    auto forward = (playerPos - pos);
    forward.Normalize();
    //プレイヤーとの距離を計算
    float dist = (playerPos - pos).Length();

    //-------------------------------
    // 攻撃種類の振り分け
    //-------------------------------
    if (dist < ATTACK_SWITCH_DISTANCE)
    {
        // 近接攻撃
        m_attacks.push_back(std::make_unique<AttackE>(
            m_deviceResources,
            pos,
            forward,
            m_displayCol
        )
        );
        //モデル
        boss->SetState(EnemyState::Attack, ATTACK_STATE_DURATION);

        m_attackCoolTimer = ATTACK_COOLDOWN;
    }
    else if (dist < DASH_SWITCH_DISTANCE)
    {
        //突進
        m_rush.push_back(std::make_unique<RushE>(
            m_deviceResources,
            boss,
            forward,
            m_displayCol
        )
        );
        //モデル
        boss->SetState(EnemyState::Rush, RUSH_STATE_DURATION);

        m_attackCoolTimer = RUSH_COOLDOWN;
    }
}

//----------------------------------------------------------
// 現在のボスのオブジェクトを取得
//----------------------------------------------------------

BossEnemy* EnemyManager::GetBoss() const
{
	//m_enemies の中から BossEnemy のインスタンスを探して返す
    for (auto& enemy : m_enemies)
    {
		//敵が生きているか
        if (auto boss = dynamic_cast<BossEnemy*>(enemy.get()))
        {
            return boss;
        }
    }
    return nullptr;
}

//----------------------------------------------------------
// 敵の中からボスのオブジェクトを取得
//----------------------------------------------------------

BossEnemy* EnemyManager::GetBossEnemy() const
{
    // アクティブな敵のリストを取得する関数を仮定
    const auto& enemies = GetActiveEnemies(); 
    
    //アクティブな敵の中からBossEnemyを探す
    for (EnemyBase* enemy : enemies)
    {
        BossEnemy* boss = dynamic_cast<BossEnemy*>(enemy);

        if (boss != nullptr)
        {
            return boss;
        }
    }
    return nullptr;
}

//----------------------------------------------------------
// 指定したHPでボスをステージに出現
//----------------------------------------------------------

void EnemyManager::SpawnBoss()
{
    // ボス生成
    auto boss = std::make_unique<BossEnemy>(
        m_deviceResources,
        DirectX::SimpleMath::Vector3(BOSS_SPAWN_POS_X, BOSS_SPAWN_POS_Y, BOSS_SPAWN_POS_Z), // 位置                                         // speed
        m_displayCol,
        EnemyType::Boss
    );

    //生成後に初期化
    boss->Initialize();

	//生成したボスを管理リストに追加
	m_enemies.push_back(std::move(boss));
}

//----------------------------------------------------------
// 通常の敵を指定した位置にステージに出現
//----------------------------------------------------------

void EnemyManager::SpawnNormalEnemy(
    const DirectX::SimpleMath::Vector3& position,
    const EnemyBaseParameter& param)
{
	//通常の敵の生成
    auto enemy = std::make_unique <NormalEnemy>(
        m_deviceResources,
        position,
        param,
        m_displayCol
	);
	//通常の敵の初期化
	enemy->Initialize();

	//生成した通常の敵を管理リストに追加
	m_enemies.push_back(std::move(enemy));
}

//----------------------------------------------------------
// 敵の近距離攻撃を生成
//----------------------------------------------------------

void EnemyManager::RequestAttackE(
    EnemyBase* enemy,
    const DirectX::SimpleMath::Vector3& forward)
{
	//近距離攻撃
    m_attacks.push_back(
        std::make_unique<AttackE>(
        m_deviceResources,
        enemy->GetPosition(),
        forward,
        m_displayCol
    )
	);
}

//----------------------------------------------------------
// 敵の突進攻撃を生成
//----------------------------------------------------------

void EnemyManager::RequestRushE(
    EnemyBase* enemy,
    const DirectX::SimpleMath::Vector3& forward)
{
	//突進
    m_rush.push_back(
        std::make_unique<RushE>(
        m_deviceResources,
        enemy,
        forward,
        m_displayCol
    )
	);
}

//----------------------------------------------------------
// 敵同士の重なりを防ぐ  総当たり
//----------------------------------------------------------

void EnemyManager::ResolveEnemyCollisions()
{
    //敵が2体未満ならチェックしない
    if (m_enemies.size() < 2)return;

    //2重ループで全ての敵の組み合わせを総当たりで確認
    for (size_t i = 0; i < m_enemies.size(); i++)
    {
        for (size_t j = i + 1; j < m_enemies.size(); j++)
        {
            auto& enemyA = m_enemies[i];
            auto& enemyB = m_enemies[j];

            //どちらかがすでにやられていたらスキップ
            if (enemyA->IsDead() || enemyB->IsDead())continue;

            //２体の現在位置を取得
            DirectX::SimpleMath::Vector3 posA = enemyA->GetPosition();
            DirectX::SimpleMath::Vector3 posB = enemyB->GetPosition();

            //高さを無視し、XZ平面で離れている方向を計算
            DirectX::SimpleMath::Vector3 diff = posB - posA;
            diff.y = 0.0f;

            float distance = diff.Length();

            //敵の当たり判定の半径を計算
            float radiusA = enemyA->GetParam().scale * 1.2f;
            float radiusB = enemyB->GetParam().scale * 1.2f;
            //重ならない最低限の距離
            float minDistance = radiusA + radiusB;

            //本来の距離より敵同士が近いか
            if (distance < minDistance)
            {
                //お互いが離れる方向ベクトル
                DirectX::SimpleMath::Vector3 pushDir = DirectX::SimpleMath::Vector3::Left;

                if (distance > VECTOR_EPSILON)
                {
                    //方向だけ
                    pushDir = diff / distance;
                }

                //重なっている長さを計算
                float overlap = minDistance - distance;
                //めり込んでいた場合、ゆっくり押し返す
                DirectX::SimpleMath::Vector3 pushAmount = pushDir * (overlap * 0.1f);

                //敵の座標を更新　
                enemyA->SetPosition(posA - pushAmount);
                enemyB->SetPosition(posB + pushAmount);
            }
        }
    }
}
