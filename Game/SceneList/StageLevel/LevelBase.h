
/**
 * @file   LevelBase.h
 * @brief  すべてのステージの親となる基底クラス
 * @author 國田知睦
 * @date   2026/06/15
 */

//ステージ共通の「親クラス」 

#pragma once
#include "pch.h"

#include "Library/StageManager.h"
#include "Game/EnemyList/EnemyManager.h"
#include "Game/GimmickList/FishManager.h"
#include "Game/GimmickList/GimmickManager.h"

#include "Game/WeatherList/WeatherBase.h"

/// <summary>
/// すべてのステージの親となる基底クラス
/// GamePlaySceneでステージ固有の処理（敵の配置など）を共通のインターフェース
/// </summary>
class LevelBase
{
public:

	virtual ~LevelBase() = default;

	/// <summary>
	/// ステージの初期化、各マネージャーを受け取りステージ固有の配置
	/// </summary>
	/// <param name="deviceResources">デバイスリソース</param>
	/// <param name="stageManager">ステージ管理クラス</param>
	/// <param name="enemyManager">敵管理クラス</param>
	/// <param name="fishManager">魚管理クラス</param>
	/// <param name="displayCollision">表示用の共通コリジョン</param>
	virtual void Initialize(
		DX::DeviceResources* deviceResources,
		StageManager* stageManager,
		EnemyManager* enemyManager,
		FishManager* fishManager,
		std::shared_ptr<DisplayCollision>displayCollision
	) 
	{
		//全ステージ共通の初期化をする
		stageManager->SetCurrentStage(L"DefaultStage");
		fishManager->SetStage(stageManager->GetCurrentStage());
		enemyManager->Initialize(deviceResources, stageManager->GetCurrentStage(), displayCollision);
	}

	/// <summary>
	/// ステージ固有の毎フレームの更新処理
	/// </summary>
	/// <param name="dt">前フレームからの経過時間</param>
	/// <param name="resources">ユーザーリソース</param>
	virtual void Update(float dt, UserResources* resources) 
	{
		//引数を使っていない
		UNREFERENCED_PARAMETER(resources);

		//天気の更新
		if (m_weather)
		{
			m_weather->Update(dt);
		}
	
	}

	
	/// <summary>
	/// ステージ固有の描画処理
	/// </summary>
	/// <param name="context"></param>
	/// <param name="view"></param>
	/// <param name="proj"></param>
	/// <param name="camPos">カメラ座標</param>
	virtual void Render(
		ID3D11DeviceContext* context, 
		const DirectX::SimpleMath::Matrix& view,
		const DirectX::SimpleMath::Matrix& proj,
		const DirectX::SimpleMath::Vector3& camPos)
	{
		//天候の描画
		if (m_weather)
		{
			m_weather->Render(context, view, proj, camPos);
		}
	}

protected:

	std::unique_ptr<WeatherBase> m_weather;
};
