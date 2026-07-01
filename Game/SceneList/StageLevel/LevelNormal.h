
/**
 * @file   LevelNormal.h
 * @brief  ノーマルステージの管理を行うクラス
 * @author 國田知睦
 * @date   2026/07/01
 */

#pragma once
#include "LevelBase.h"
#include "Game/EnemyList/EnemyBaseParameter.h"
#include "Game/WeatherList/Rain.h"
#include "Game/WeatherList/Snow.h"
#include "Game/SoundList/AudioManager.h"

/// <summary>
/// ノーマルステージの管理を行うクラス
/// </summary>
class LevelNormal :public LevelBase
{
private:

    //敵の出現位置
    static constexpr float SPAWN_RANGE_MIN = -20.0f;
    static constexpr float SPAWN_RANGE_MAX = 20.0f;
    //敵の数
    static constexpr int ENEMY_COUNT = 2;

    //BGMの大きさ
    static constexpr float DEFAULT_BGM_VOLUME = 0.2f;

    //画面サイズ
    static constexpr int SCREEN_SIZE_WIDTH = 1280;
    static constexpr int SCREEN_SIZE_HEIGHT = 720;

public:

	/// <summary>
	/// ノーマルステージの初期化
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
        std::shared_ptr<DisplayCollision> /*displayCollision*/
	)override
	{
        //ステージ共通の初期化
        //LevelBase::Initialize(deviceResources, stageManager, enemyManager, fishManager, displayCollision);

        //---------------------------------------
        // 個別のステージの登録
        //---------------------------------------

        stageManager->AddStage(L"NormalStage",
            deviceResources->GetWindow(),
            SCREEN_SIZE_WIDTH, SCREEN_SIZE_HEIGHT,
            "Resources\\Stages\\NormalStage.bmp");

        stageManager->SetCurrentStage(L"NormalStage");

        //天候　2択
        if (rand() % 2 == 0)
        {
            //雨
            m_weather = std::make_unique<Rain>();
        }
        else
        {
            //雪
            m_weather = std::make_unique<Snow>();
        }
        //天候の初期化
        m_weather->Initialize(deviceResources->GetD3DDevice());

        //ステージのBGM
        AudioManager* audio = AudioManager::GetInstance();
        audio->LoadSound("Stage1_BGM", L"Resources/Sounds/BGM_Game.wav");
        audio->SetBGMVolume(DEFAULT_BGM_VOLUME); 
        audio->PlayBGM("Stage1_BGM");

        //乱数
        std::random_device rd;
        std::mt19937 gen(rd());
        //X,Zの範囲を指定
        std::uniform_real_distribution<float> disX(SPAWN_RANGE_MIN, SPAWN_RANGE_MAX);
        std::uniform_real_distribution<float> disZ(SPAWN_RANGE_MAX, SPAWN_RANGE_MIN);

        //ループしてランダムな座標に入れていく
        for (int i = 0; i < ENEMY_COUNT; i++)
        {
            DirectX::SimpleMath::Vector3 randomPos(
                disX(gen),
                0.0f,
                disZ(gen)
            );

            //敵の呼び出し
            enemyManager->SpawnNormalEnemy(randomPos,EnemyData::BigNormalEnemy);
        }

        //---------------------------------------
        // 魚（ギミック）の配置
        //---------------------------------------
        
        // 魚配置
        fishManager->SetStage(stageManager->GetCurrentStage());
	}

    /// <summary>
    /// ノーマルステージの更新
    /// </summary>
    /// <param name="dt">前フレームからの経過時間</param>
    /// <param name="resources">ユーザーリソース</param>
    void Update(float dt, UserResources* resources) override
    {
        LevelBase::Update(dt, resources);
    }
};