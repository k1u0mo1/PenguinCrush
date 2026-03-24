

#include "pch.h"
#include "EnemyManager.h"
#include <PlayerList/Player.h>

#include <algorithm>

#include <Model.h>   
#include <Effects.h>

#include "Effects/Particle.h"
#include <SoundList/AudioManager.h>


using namespace DirectX;

EnemyManager::EnemyManager()
{
}

EnemyManager::~EnemyManager()
{
}

void EnemyManager::Initialize(
	DX::DeviceResources* deviceResources,
	Stage* stage,
    std::shared_ptr<DisplayCollision> displayCollision)
{

    m_deviceResources = deviceResources;
    m_stage = stage;

    m_displayCol = displayCollision;

    //ステートの作成
    m_states = std::make_unique<DirectX::CommonStates>(deviceResources->GetD3DDevice());

    DirectX::EffectFactory fx(deviceResources->GetD3DDevice());

    fx.SetDirectory(L"Resources\\Models");
    m_bulletModel = Model::CreateFromSDKMESH(
        deviceResources->GetD3DDevice(),
        L"Resources\\Models\\fish.sdkmesh",
        fx
    );

}

void EnemyManager::Update(
    float dt,  Player* player, Particle* particle)
{

    if (!m_boss) return;

    //敵が生きている
    if (m_boss)
    {
        //更新
        m_boss->Update(
            dt,
            player->GetPosition(),
            player->GetMoveSpeed(),
            m_stage,
            particle
        );

        //敵がLoadingなら止まる
        if (m_boss->GetState() == BossEnemy::EnemyState::Loading)
        {
            return;
        }

        //生きている
        if(!m_boss->IsDead())
        {
            //攻撃生成
            DoBossAttack(dt, player->GetPosition());
        }
    }

    //--------------------------------
    //弾・攻撃の更新 
    //--------------------------------
    for (size_t i = 0; i < m_attacks.size(); i++)
    {
        m_attacks[i]->Update(dt);
    }
    for (size_t i = 0; i < m_bullets.size(); i++)
    {
        m_bullets[i]->Update(dt,m_stage);
    }
    for (size_t i = 0; i < m_rushs.size(); i++)
    {
        m_rushs[i]->Update(dt);

        // 突進エフェクト
        if (particle && m_boss)
        {
            // 現在のボスの位置を取得
            DirectX::SimpleMath::Vector3 pos = m_boss->GetPosition();
            
            pos.y += 1.0f;
           
            particle->Spawn(Particle::Type::Dash, pos, 1, 0.3f);
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
                    hitPos.y += 1.0f;

                    //エフェクト生成
                    particle->Spawn(Particle::Type::Explosion, hitPos, 10,0.1);
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
        for (auto& rush : m_rushs)
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
                    hitPos.y += 1.0f;

                    // エフェクト生成
                    particle->Spawn(Particle::Type::Explosion, hitPos, 10,0.1);
                }

                //効果音
                AudioManager::GetInstance()->Play("DashE");

                //ダメージ適用
                player->TakeDamage(rush->GetDamage());

                //攻撃が当たったら削除
                rush->SetDead();
            }
        }

        //Bullet（弾）の衝突判定
        for (auto& bullet : m_bullets)
        {
            //攻撃コリジョンとプレイヤーのコリジョンの衝突判定
            if (bullet->GetCollision()->Intersects(player->GetCollision()))
            {
                //プレイヤーにノックバックとダメージ
                player->ApplyKnockback(bullet->GetForward(), bullet->GetKnockbackPower());

                if (particle)
                {
                    // プレイヤーの位置を取得
                    DirectX::SimpleMath::Vector3 hitPos = player->GetPosition();

                    //高さを少し上げる
                    hitPos.y += 1.0f;

                    //エフェクト生成
                    particle->Spawn(Particle::Type::Explosion, hitPos, 10,0.1);
                }

                //効果音
                AudioManager::GetInstance()->Play("DashE");

                //ダメージ適用
                player->TakeDamage(bullet->GetDamage());

                //攻撃が当たったら削除
                bullet->SetDead();
            }
        }
        
    }
    
    


    //--------------------------------
    // 生存判定：死んだものを削除
    //--------------------------------
    m_attacks.erase(
        std::remove_if(m_attacks.begin(), m_attacks.end(),
            [](std::unique_ptr<AttackE>& a) { return a->IsDead(); }),
        m_attacks.end());

    m_bullets.erase(
        std::remove_if(m_bullets.begin(), m_bullets.end(),
            [](std::unique_ptr<BulletE>& b) { return !b->IsAlive(); }),
        m_bullets.end());

    m_rushs.erase(
        std::remove_if(m_rushs.begin(), m_rushs.end(),
            [](std::unique_ptr<RushE>& r) { return r->IsDead(); }),
        m_rushs.end());

}

void EnemyManager::Render(
    ID3D11DeviceContext* context,
    const DirectX::SimpleMath::Matrix& view,
    const DirectX::SimpleMath::Matrix& proj,
    ShadowRenderer* shadowRenderer
)
{
    //影を付ける
    if (m_boss && !m_boss->IsDead() && shadowRenderer)
    {
        //ボスの現在位置を取得
        DirectX::SimpleMath::Vector3 shadowPos = m_boss->GetPosition();

        
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

    //敵の描画
    if (m_boss && !m_boss->IsDead())
    {
        m_boss->Render(context, view, proj);
    }


    for (auto& atk : m_attacks)
    {
        atk->Render(context, view, proj);
    }
    for (auto& b : m_bullets)
    {
        b->Render(context, view, proj);
    }
    for (auto& r : m_rushs)
    {
        r->Render(context, view, proj);
    }
}

std::vector<BossEnemy*> EnemyManager::GetActiveEnemies()const
{
    std::vector<BossEnemy*> enemies;
    if (m_boss && !m_boss->IsDead())
    {
        enemies.push_back(m_boss.get());
    }
    return enemies;
}

void EnemyManager::DoBossAttack(
    float dt, 
    const DirectX::SimpleMath::Vector3& playerPos
    )
{
    if (!m_boss) return;

    m_attackCoolTimer -= dt;

    //クールタイマ
    if (m_attackCoolTimer > 0)
    {
        return;
    }
    //敵の座標をもらう
    auto pos = m_boss->GetPosition();
    auto forward = (playerPos - pos);
    forward.Normalize();

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
        m_boss->SetState(BossEnemy::EnemyState::Attack, ATTACK_STATE_DURATION);

        m_attackCoolTimer = ATTACK_COOLDOWN;
    }
    else if (dist < DASH_SWITCH_DISTANCE)
    {
        //突進
        m_rushs.push_back(std::make_unique<RushE>(
            m_deviceResources,
            m_boss.get(),
            forward,
            m_displayCol
        )
        );
        //モデル
        m_boss->SetState(BossEnemy::EnemyState::Rush, RUSH_STATE_DURATION);

        m_attackCoolTimer = RUSH_COOLDOWN;

    }
    else
    {
        //弾
        auto spawnPos = pos + forward * BULLET_SPAWN_DISTANCE + DirectX::SimpleMath::Vector3(0, BULLET_SPAWN_HEIGHT, 0);

        auto bullet = std::make_unique<BulletE>(
            m_deviceResources,
            spawnPos,
            forward,
            m_displayCol,
            m_bulletModel
        );

        //モデル
        m_boss->SetState(BossEnemy::EnemyState::Shoot, SHOOT_STATE_DURATION);

        m_bullets.push_back(std::move(bullet));
        m_attackCoolTimer = SHOOT_COOLDOWN;

        //効果音
        AudioManager::GetInstance()->Play("BulletE");
    }
}

BossEnemy* EnemyManager::GetBossEnemy() const
{
    const auto& enemies = GetActiveEnemies(); // アクティブな敵のリストを取得する関数を仮定
    if (!enemies.empty())
    {
        return enemies[0];
    }
    return nullptr;
}

void EnemyManager::SpawnBoss(float hp)
{
    // ボス生成
    m_boss = std::make_unique<BossEnemy>(
        m_deviceResources,
        DirectX::SimpleMath::Vector3(10, 50.0f, 10), // 位置
        hp,                                          // HP
        5.0f,                                        // speed
        m_displayCol
    );

    //生成後に初期化
    m_boss->Initialize();
}
