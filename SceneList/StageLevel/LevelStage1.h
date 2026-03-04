#pragma once
#include "LevelBase.h"

class LevelStage1 :public LevelBase
{
public:

	void Initialize(
        DX::DeviceResources* deviceResources,
		StageManager* stageManager,
		EnemyManager* enemyManager,
		FishManager* fishManager,
		/*GimmickManager* gimmickManager,*/
        std::shared_ptr<DisplayCollision> displayCollision
	)override
	{
        //---------------------------------------
        //  ステージ（地面）の設定
        //---------------------------------------
        // とりあえず既存のコードに合わせてセット
        stageManager->SetCurrentStage(L"DefaultStage");

        fishManager->SetStage(stageManager->GetCurrentStage());

        //---------------------------------------
        // 敵の配置
        //---------------------------------------
        // 敵の初期化（リセット）
        enemyManager->Initialize(
            deviceResources,
            stageManager->GetCurrentStage(),
            displayCollision
        );

        //ボスを出す　hp設定
        enemyManager->SpawnBoss(1500.0f);

        //---------------------------------------
        // 魚（ギミック）の配置
        //---------------------------------------
        
        // 魚配置
        fishManager->SpawnRandomFish(1);
	}

    void Update(float /*dt*/, UserResources* /*resources*/) override
    {
        //ステージ1固有の処理があればここに書く
    }
};