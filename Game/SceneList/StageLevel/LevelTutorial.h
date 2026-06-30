
/**
 * @file   LevelTutorial.h
 * @brief  チュートリアルステージの管理とUI表示を行うクラス
 * @author 國田知睦
 * @date   2026/06/22
 */


#pragma once
#include "LevelBase.h"

//テクスチャを出す用
#include "WICTextureLoader.h"
#include "SpriteBatch.h"
#include "CommonStates.h"
#include "Game/EnemyList/EnemyBaseParameter.h"

#include "Game/WeatherList/Rain.h"
#include "Game/WeatherList/Snow.h"
#include "Game/SoundList/AudioManager.h"

/// <summary>　
/// チュートリアルステージの管理とUI表示を行うクラス
/// </summary>
class LevelTutorial :public LevelBase
{
private:

	//BGMの大きさ
	static constexpr float DEFAULT_BGM_VOLUME = 0.2f;

	//敵が出る座標
	static constexpr float ENEMY_POS_X = 10.0f;
	static constexpr float ENEMY_POS_Z = 10.0f;

	//表示フラグ
	bool m_isShowHelp = true;

	//操作方法UI
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_textureUI;
	
	//描画用バッチ
	std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;
	//ステート
	std::unique_ptr<DirectX::CommonStates> m_states;

public:

	/// <summary>
	/// チュートリアルレベルの初期化
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
		std::shared_ptr<DisplayCollision> /*displayCollision*/
	)override
	{
		auto device = deviceResources->GetD3DDevice();
		auto context = deviceResources->GetD3DDeviceContext();

		//作成
		m_spriteBatch = std::make_unique<DirectX::SpriteBatch>(context);
		m_states = std::make_unique<DirectX::CommonStates>(device);

		//テクスチャの読み込み
		DirectX::CreateWICTextureFromFile(
			device,
			L"Resources\\Textures\\TutorialUI.png",
			nullptr,
			m_textureUI.GetAddressOf()
		);

		//ステージ共通の初期化
		//LevelBase::Initialize(deviceResources, stageManager, enemyManager, fishManager, displayCollision);

		//---------------------------------------
		// 個別のステージの登録
		//---------------------------------------

		stageManager->AddStage(L"TutorialStage",
			deviceResources->GetWindow(),
			1280, 720,
			"Resources\\Stages\\TutorialStage.bmp");

		stageManager->SetCurrentStage(L"TutorialStage");



		//天候　2択
		if (rand() % 2 == 0)
		{
			m_weather = std::make_unique<Rain>();
		}
		else
		{
			m_weather = std::make_unique<Snow>();
		}

		//天候の初期化
		m_weather->Initialize(deviceResources->GetD3DDevice());

		//ステージのBGM
		AudioManager* audio = AudioManager::GetInstance();
		audio->LoadSound("Stage1_BGM", L"Resources/Sounds/BGM_Game.wav");
		audio->SetBGMVolume(DEFAULT_BGM_VOLUME);
		audio->PlayBGM("Stage1_BGM");

		//敵の出現座標
		const DirectX::SimpleMath::Vector3 enemySpawnPos(ENEMY_POS_X, 0.0f, ENEMY_POS_Z);

		//敵の出現位置と敵の種類
		enemyManager->SpawnNormalEnemy(enemySpawnPos,EnemyData::NormalEnemy);

		
		//---------------------------------------
		// 魚（ギミック）の配置
		//---------------------------------------

		fishManager->SetStage(stageManager->GetCurrentStage());

	}

	/// <summary>
	/// チュートリアルレベルの更新
	/// </summary>
	/// <param name="dt">前フレームからの経過時間</param>
	/// <param name="resources">ユーザーリソース</param>
	void Update(float /*dt*/, UserResources* resources) override
	{
		auto input = resources->GetInputManager();

		// [2]キーを押すと、表示/非表示を反転させる
		if (input->kbTracker.pressed.D3)
		{
			m_isShowHelp = !m_isShowHelp;
		}
	}

	
	/// <summary>
	/// チュートリアルレベルの描画
	/// </summary>
	/// <param name="context"></param>
	/// <param name="view"></param>
	/// <param name="proj"></param>
	/// <param name="camPos"></param>
	void Render(
		ID3D11DeviceContext* context,
		const DirectX::SimpleMath::Matrix& view,
		const DirectX::SimpleMath::Matrix& proj,
		const DirectX::SimpleMath::Vector3& camPos
	) override
	{
		UNREFERENCED_PARAMETER(context);
		UNREFERENCED_PARAMETER(view);
		UNREFERENCED_PARAMETER(proj);
		UNREFERENCED_PARAMETER(camPos);
		

		//フラグがfalseなら何も描画しない
		if (!m_isShowHelp) return;

		//テクスチャの描画
		if (m_spriteBatch && m_textureUI)
		{
			m_spriteBatch->Begin(
				DirectX::SpriteSortMode_Deferred,
				m_states->NonPremultiplied()
			);

			//描画
			m_spriteBatch->Draw(m_textureUI.Get(), DirectX::SimpleMath::Vector2(0, 0));

			m_spriteBatch->End();

		}
	}
};

