
//魚のマネージャー

#include "pch.h"
#include "Game/GimmickList/FishManager.h"
#include "Game/PlayerList/Player.h"

#include "Game/ShadowRenderer/ShadowRenderer.h"

using namespace DirectX;

//----------------------------------------------------------
// インスタンスを生成
//----------------------------------------------------------

FishManager::FishManager(
    DX::DeviceResources* deviceResources,
    std::shared_ptr<DisplayCollision> displayCollision,
    Stage* stage)
    :
    m_deviceResources(deviceResources),
    m_displayCollision(displayCollision),
    m_stage(stage)
{
}

//----------------------------------------------------------
// 魚のスポーンタイマーの更新とプレイヤーとの当たり判定
//----------------------------------------------------------

void FishManager::Update(float dt, Player* player)
{
    m_spawnTimer += dt;
    if (m_spawnTimer >= FISH_SPAWN_INTERVAL)
    {
        m_spawnTimer = 0.0f;
        SpawnFish();
    }

    for (auto& fish : m_fish)
    {
        fish->Update(dt, m_stage);

        if (!fish->IsAlive()) continue;

        auto playerCol = player->GetCollision();
        BoundingSphere col = fish->GetBoundingSphere();

        if (playerCol &&
            playerCol->GetType() == ModelCollision::CollisionType::OBB)
        {
            auto* obbCol =
                dynamic_cast<ModelCollisionOrientedBox*>(playerCol);

            for (auto& obb : obbCol->GetBoundingInfo())
            {
                if (col.Intersects(obb))
                {
                    //弾数の回復
                    player->AddAmmo(fish->GetAmmoValue());
                    
                    //HPを回復
                    player->Heal(FISH_HEAL_AMOUNT);

                    //弾を消す
                    fish->BulletKill();

                    break;
                }
            }
        }
    }

    // 死んだ魚を削除
    m_fish.erase(
        std::remove_if(
            m_fish.begin(), m_fish.end(),
            [](auto& f) { return !f->IsAlive(); }),
        m_fish.end());
}

//----------------------------------------------------------
// 存在しているすべての魚の描画
//----------------------------------------------------------

void FishManager::Render(ID3D11DeviceContext* ctx,
    const DirectX::SimpleMath::Matrix& view,
    const DirectX::SimpleMath::Matrix& proj,
    ShadowRenderer* shadowRenderer)
{
    for (auto& fish : m_fish)
        fish->Render(ctx, view, proj,shadowRenderer,m_stage);
}

//----------------------------------------------------------
// 指定した数の魚をステージ上にランダムな位置にスポーン
//----------------------------------------------------------

void FishManager::SpawnRandomFish(int count)
{
    for (int i = 0; i < count; ++i)
    {
        SpawnFish(); 
    }
}

//----------------------------------------------------------
// 魚のスポーン
//----------------------------------------------------------

void FishManager::SpawnFish()
{
    // ランダム座標
    float x = static_cast<float>(rand() % SPAWN_AREA_SIZE - SPAWN_AREA_OFFSET);
    float z = static_cast<float>(rand() % SPAWN_AREA_SIZE - SPAWN_AREA_OFFSET);

    // ステージの高さ
    float y = m_stage->GetGroundHeight(x, z) + SPAWN_HEIGHT_OFFSET;

    auto fish = std::make_unique<Fish>(
        m_deviceResources,
        SimpleMath::Vector3(x, y, z),
        SimpleMath::Vector3(0.001f, -1.0f, 0), 
        m_displayCollision
    );

    //// Fish の初期化
    //fish->Initialize(
    //    m_deviceResources->GetWindow(),
    //    m_deviceResources->GetOutputSize().right,
    //    m_deviceResources->GetOutputSize().bottom
    //);

    fish->Initialize();

    m_fish.push_back(std::move(fish));
}