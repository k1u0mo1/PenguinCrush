
//ステージ共通の「親クラス」 

#pragma once
#include "pch.h"

#include "Library/StageManager.h"
#include "EnemyList/EnemyManager.h"
#include "GimmickList/FishManager.h"
#include "Library/GimmickManager.h"

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
		/*GimmickManager* gimmickManager,*/
		std::shared_ptr<DisplayCollision>displayCollision
	) = 0;

	/// <summary>
	/// ステージ固有の毎フレームの更新処理
	/// </summary>
	/// <param name="">前フレームからの経過時間</param>
	/// <param name="">ユーザーリソース</param>
	virtual void Update(float /*dt*/, UserResources* /*resources*/) {}

	/// <summary>
	/// ステージ固有の描画処理
	/// </summary>
	/// <param name="">使用していない</param>
	virtual void Render(UserResources* /*resources*/){}
};
