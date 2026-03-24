//ゲームプレイシーンクラス//////////////////////////////

#pragma once

//使用クラス----------------

#include "pch.h"
#include "UserResources.h"

//シーン管理
#include "Library/SceneManager.h"
//ステージ管理
#include "Library/StageManager.h"
//アタック管理
#include "PlayerList/AttackList/AttackManager.h"

//Player------------------------
#include "PlayerList/Player.h"

//ギミック系のクラス------------

#include "Library/GimmickManager.h"
//波
#include "GimmickList/Wave.h"
//魚
#include "GimmickList/FishManager.h"

//オブジェクト------------------
//スカイドーム
#include "ObjectList/SkyDome.h"

//カメラ系-------------------------
//デバッグ用カメラ
#include "Camera/DebugCamera.h"
//プレイヤー用カメラ
#include "Camera/PlayerCamera.h"

//敵系-------------------------
#include "EnemyList/EnemyManager.h"

//UI------------------------
#include "UIList/GameUI.h"

//Stage --------------
#include "SceneList/StageLevel/LevelBase.h"

//天候-------------------------------------------
#include "WeatherList/WeatherBase.h"

//シェーダ関連----------------------------------
#include "Effects/Particle.h"

//影-----------------------------------------
#include "ShadowRenderer/ShadowRenderer.h"


class GamePlayScene : public Scene<UserResources>
{
	//カメラのタイプ
	enum  class CameraType
	{
		Debug,
		Player
	};

private:

	//敵の登場時間
	static constexpr float OPENING_DURATION = 3.0f;

public:
	//ステージの種類
	enum class StageType
	{
		Tutorial,
		Stage1,
		Stage2
	};

	/// <summary>
	/// 次に読み込むステージの種類を指定
	/// SelectScene でステージを選択した際にセット
	/// </summary>
	static StageType NextStageType;

private:
	//現在のレベル管理クラス
	std::unique_ptr<LevelBase> m_currentLevel;

private:
	//初期状態
	CameraType m_currentCamera = CameraType::Player;

	//継承シーン関数
public:

	//コンストラクタ
	GamePlayScene();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize()override;

	/// <summary>
	/// 更新
	/// </summary>
	/// <param name="elapsedTime">前フレームからの経過時間</param>
	void Update(float elapsedTime)override;

	/// <summary>
	/// 描画
	/// </summary>
	void Render()override;

	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize()override;

	/// <summary>
	/// デバイスに依存するリソースを作成する関数
	/// </summary>
	void CreateDeviceDependentResources() override;

	/// <summary>
	/// ウインドウサイズに依存するリソースを作成する関数
	/// </summary>
	void CreateWindowSizeDependentResources() override;

	/// <summary>
	/// デバイスロストした時に呼び出される関数
	/// </summary>
	void OnDeviceLost() override;

	/// <summary>
	/// シーン系をまとめる
	/// </summary>
	void SceneChange();

	/// <summary>
	/// カメラの更新をまとめる
	/// </summary>
	void UpdateCamera();
	
private:

	//ビュー行列
	DirectX::SimpleMath::Matrix m_view;

	//射影行列
	DirectX::SimpleMath::Matrix m_proj;

	//モデルポインタ
	std::unique_ptr<DirectX::CommonStates> m_states;
private:

	//現在は使用していない///////////////////
	//デバックUP関連
	void DebugUpdate(float timer);

	//デバック描画関連
	void DebugRender();
	/////////////////////////////////////////

	//デバック描画用
	DirectX::SimpleMath::Vector3 m_playerPos;
	DirectX::SimpleMath::Vector3 m_enemyPos;
	DirectX::SimpleMath::Vector3 m_stagePos;

private:
	//リソース
	DX::DeviceResources* m_deviceResources;

	//デバッグフォント
	std::unique_ptr<DebugFont> m_debugFont;

	//プレイヤー
	std::unique_ptr<Player> m_player;

	//アタックマネージャー
	std::unique_ptr<AttackManager> m_attackManager;

	//ギミック
	std::unique_ptr<GimmickManager> m_gimmickManager;

	std::unique_ptr<StageManager> m_stageManager;

	//敵関連マネージャー
	std::unique_ptr<EnemyManager> m_enemyManager;

	//波
	std::unique_ptr<Wave> m_wave;

	//魚
	std::unique_ptr<FishManager> m_fishManager;

	//天候
	std::unique_ptr<WeatherBase> m_weather;

	//湧き時間
	float m_fishSpawnTimer = 0.0f;

	//カメラ（デバッグ用）
	std::unique_ptr<DebugCamera> m_debugCamera;

	//カメラ（プレイヤー用）
	std::unique_ptr<PlayerCamera> m_playerCamera;

	//スカイドーム
	std::unique_ptr<SkyDome> m_skyDome;

	//ゲームのUI（プレイヤーと敵）
	std::unique_ptr<GameUI> m_gameUI;

	//ゲームが終わったかどうか
	bool m_isFinished ;

	//終了時の待機タイマー
	float m_finishTimer;

	//デバック表示/非表示
	bool m_isDebugVisible = true;

	//コリジョンをオン/オフ
	std::shared_ptr<DisplayCollision> m_displayCollision;

	//一時停止（ポーズ）中かどうかのフラグ
	bool m_isPaused;

	//シェーダ用
	std::unique_ptr<Particle> m_particle;

	//登場演出　タイマー
	float m_openingTimer = 0.0f;

	//オープニング中か
	bool m_isOpening = true;

	std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch; 

	// ボタンUIテクスチャ
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_textureButtonUI1;
	// ボタンUIテクスチャ
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_textureButtonUI2;

private:
	//シーン移動中のフラグ
	bool m_isChangingScene = false;

	//オーディオマネージャー
	std::unique_ptr<AudioManager> m_audioManager;

	//影
	std::unique_ptr<ShadowRenderer> m_shadowRenderer;
};


