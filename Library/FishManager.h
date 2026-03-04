
#pragma once
#include "pch.h"
#include <Common/DeviceResources.h>
#include <Collision/DisplayCollision.h>
#include <PlayerList/Player.h>

#include "GimmickList/Stage.h"
#include "GimmickList/Fish.h"

class ShadowRenderer;

class FishManager
{
public:
    FishManager(
        DX::DeviceResources* deviceResources,
        std::shared_ptr<DisplayCollision> displayCollision,
        Stage* stage);

    void Update(float elapsedTime, Player* player);

    void Render(ID3D11DeviceContext* context,
        const DirectX::SimpleMath::Matrix& view,
        const DirectX::SimpleMath::Matrix& proj,
        ShadowRenderer* shadowRenderer);

    void SpawnRandomFish(int count);

    //ステージ情報を更新
    void SetStage(Stage* stage) { m_stage = stage; }

private:

    void SpawnFish();

private:
    DX::DeviceResources* m_deviceResources;
    std::shared_ptr<DisplayCollision> m_displayCollision;
    Stage* m_stage;

    float m_spawnTimer = 0.0f;
    std::vector<std::unique_ptr<Fish>> m_fish;
};