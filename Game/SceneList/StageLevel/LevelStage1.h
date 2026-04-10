#pragma once
#include "LevelBase.h"

/// <summary>
/// ステージ１の管理を行うクラス
/// </summary>
class LevelStage1 :public LevelBase
{
public:

	/// <summary>
	/// ステージ１レベルの初期化
	/// </summary>
	/// <param name="deviceResources">デバイスリソース</param>
	/// <param name="stageManager">ステージマネージャー</param>
	/// <param name="enemyManager">エネミーマネージャー</param>
	/// <param name="fishManager">魚（ギミック）マネージャー</param>
	/// <param name="displayCollision">当たり判定の表示用</param>
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
        enemyManager->SpawnBoss(2000.0f);

        //---------------------------------------
        // 魚（ギミック）の配置
        //---------------------------------------
        
        // 魚配置
        fishManager->SpawnRandomFish(1);
	}

    /// <summary>
    /// ステージ１レベルの更新
    /// </summary>
    /// <param name="dt">前フレームからの経過時間</param>
    /// <param name="resources">ユーザーリソース</param>
    void Update(float /*dt*/, UserResources* /*resources*/) override
    {
        //ステージ1固有の処理があればここに書く
    }
};