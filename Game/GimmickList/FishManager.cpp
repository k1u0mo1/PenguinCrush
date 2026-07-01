/**
 * @file   FishManager.cpp
 * @brief  魚の描画・管理を行うマネージャークラス
 * @author 國田知睦
 * @date   2026/07/01
 */

#include "pch.h"
#include "Game/GimmickList/FishManager.h"
#include "Game/PlayerList/Player.h"
#include "Game/ShadowRenderer/ShadowRenderer.h"

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
    //魚のスポーンタイマーを更新
    m_spawnTimer += dt;
    if (m_spawnTimer >= FISH_SPAWN_INTERVAL)
    {
        m_spawnTimer = 0.0f;
        SpawnFish();
    }
    //魚の更新とプレイヤーとの当たり判定
    for (auto& fish : m_fish)
    {
        //魚の更新
        fish->Update(dt, m_stage);
        //生きていなければ当たり判定を行わない
        if (!fish->IsAlive()) continue;

        //プレイヤーのコリジョンを取得
        auto playerCol = player->GetCollision();
        DirectX::BoundingSphere col = fish->GetBoundingSphere();

        //プレイヤーのコリジョンOBB型なら当たり判定を行う
        if (playerCol &&
            playerCol->GetType() == ModelCollision::CollisionType::OBB)
        {
            auto* obbCol =
                dynamic_cast<ModelCollisionOrientedBox*>(playerCol);
            //魚の当たり判定とプレイヤーの当たり判定を比較
            for (auto& obb : obbCol->GetBoundingInfo())
            {
                if (col.Intersects(obb))
                {
                    //HPを回復
                    player->Heal(FISH_HEAL_AMOUNT);
                    //回復音を再生
                    AudioManager::GetInstance()->Play("Heal");
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
        DirectX::SimpleMath::Vector3(x, y, z),
        DirectX::SimpleMath::Vector3(DEFAULT_DIR_X, DEFAULT_DIR_Y, DEFAULT_DIR_Z),
        m_displayCollision
    );

    fish->Initialize();

    m_fish.push_back(std::move(fish));
}