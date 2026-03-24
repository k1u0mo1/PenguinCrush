
//ゲームシーンクラス

#include "pch.h"
#include "GamePlayScene.h"

//ゲームプレイシーンー＞リザルトシーンに移動
#include "ResultScene.h"

#include "SelectScene.h"

#include "TitleScene.h"

//どのステージ
#include "SceneList/StageLevel/LevelBase.h"
#include "SceneList/StageLevel/LevelTutorial.h"
#include "SceneList/StageLevel/LevelStage1.h"
#include "SceneList/StageLevel/LevelStage2.h"

#include <sstream> 
#include <iomanip>
#include <WeatherList/Rain.h>
#include <WeatherList/Snow.h>
#include "SoundList/AudioManager.h"

using namespace DirectX;

GamePlayScene::StageType GamePlayScene::NextStageType =
GamePlayScene::StageType::Stage1;

//-----------------------------------------------------------------
//コンストラクタ
//-----------------------------------------------------------------
GamePlayScene::GamePlayScene()
	: m_deviceResources{}
	, m_isFinished(false)
	, m_finishTimer(3.0f)
	, m_isDebugVisible(false)
	, m_isPaused(false)
{
}

//-----------------------------------------------------------------
//初期化関連
//-----------------------------------------------------------------
void GamePlayScene::Initialize()
{
	//デバイスリソースを取得
	m_deviceResources = GetUserResources()->GetDeviceResources();

	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();

	//終わったか
	m_isFinished = false;

	//終わったときに何秒後に
	m_finishTimer = 3.0f;

	//デバックを表示・非表示
	m_isDebugVisible = false;

	//現在、一時停止中か
	m_isPaused = false;


	m_isOpening = true;

	//演出の時間
	m_openingTimer = 0.0f;

	//シーン移動中
	m_isChangingScene = false;

	//オープニング演出のセットアップ
	auto boss = m_enemyManager->GetBossEnemy();
	if (boss != nullptr)
	{
		boss->SetState(BossEnemy::EnemyState::Loading, 3.0f);
	}

	//フェードイン開始
	GetUserResources()->GetTransitionMask()->Open();

	//音量設定 
	AudioManager::GetInstance()->SetSEVolume(0.2f);
}

//-----------------------------------------------------------------
//更新関係
//-----------------------------------------------------------------
void GamePlayScene::Update(float elapsedTime)
{

	//ボタン
	auto input = GetUserResources()->GetInputManager();

	//フェード
	auto transitionMask = GetUserResources()->GetTransitionMask();

	//BGMの更新
	AudioManager::GetInstance()->Update();

	//-------------------------------------------------
	//ボタン系
	//-------------------------------------------------
	//[P]で切り替える
	if (input->kbTracker.pressed.P)
	{
		m_isPaused = !m_isPaused;
	}

	//一時停止
	if (m_isPaused)
	{
		return;
	}

	//-------------------------------------------------
	//シーン
	//-------------------------------------------------

	SceneChange();

	//-------------------------------------------------
	//敵登場の更新
	//-------------------------------------------------
	if (m_isOpening)
	{
		m_openingTimer += elapsedTime;

		if (m_openingTimer >= OPENING_DURATION)
		{
			m_isOpening = false;

			//敵が存在するかチェックする 
			auto boss = m_enemyManager->GetBossEnemy();
			if (boss != nullptr)
			{
				boss->SetState(BossEnemy::EnemyState::Idle, 0.0f);

				//タイマーをリセット
				m_openingTimer = 0.0f;
			}
		}
	}


	//ステージの情報を渡す
	if (m_currentLevel)
	{
		m_currentLevel->Update(elapsedTime, GetUserResources());
	}

	//-------------------------------------------------
	//勝敗の更新
	//-------------------------------------------------
	if (m_isFinished)
	{
		m_finishTimer -= elapsedTime;
		if (m_finishTimer <= 0.0f)
		{
			ChangeScene<ResultScene>();
		}
		return;
	}

	//プレイヤーがやられたとき
	if (m_player && m_player->IsDead())
	{
		//終了フラグ
		m_isFinished = true;
		//「負け」
		GetUserResources()->SetGameClear(false);

		m_isChangingScene = true;
		//フェード演出
		transitionMask->Close();
		return;
	}

	//ボスがやられたとき
	if (m_enemyManager && m_enemyManager->HasBoss())
	{
		BossEnemy* boss = m_enemyManager->GetBoss();
		//ボスが完全にやられた
		if (boss && boss->IsDead())
		{
			//終了フラグ
			m_isFinished = true;
			//「勝ち」
			GetUserResources()->SetGameClear(true);

			m_isChangingScene = true;
			//フェード演出
			transitionMask->Close();
			return;
		}
	}

	//-------------------------------------------------
	//ギミックの更新
	//-------------------------------------------------

	if (m_gimmickManager)
	{
		m_gimmickManager->Update(elapsedTime);
	}

	//-------------------------------------------------
	//ステージの更新（波）
	//-------------------------------------------------

	//波を取得
	auto wave = m_gimmickManager->Get<Wave>();

	if (m_stageManager)
	{
		m_stageManager->Update(wave.get());
	}
	

	// パーティクルの更新を追加
	if (m_particle)
	{
		m_particle->Update(elapsedTime);
	}

	//-------------------------------------------------
	//天候の更新
	//-------------------------------------------------

	// 天候の更新
	if (m_weather)
	{
		m_weather->Update(elapsedTime);
	}

	//-------------------------------------------------
	//プレイヤーの更新
	//-------------------------------------------------
	if (m_player && !m_player->IsDead())
	{
		m_player->Update(
			elapsedTime,
			input->mouse,
			input->mouseTracker,
			m_stageManager->GetCurrentStage(),
			wave.get(),
			m_particle.get()
		);
	}

	//-------------------------------------------------
	//カメラの更新
	//-------------------------------------------------

	UpdateCamera();

	//-------------------------------------------------
	//魚の更新
	//-------------------------------------------------

	// 魚描画
	if(m_fishManager && m_player)
	{
		m_fishManager->Update(elapsedTime, m_player.get());
	}

	//-------------------------------------------------
	//AttackManagerの更新
	//-------------------------------------------------
	if (m_attackManager)
	{
		std::vector<BossEnemy*> activeEnemies = m_enemyManager->GetActiveEnemies();
		m_attackManager->Update(elapsedTime, activeEnemies, m_particle.get());
	}

	//-------------------------------------------------
	//敵の更新
	//-------------------------------------------------
	m_enemyManager->Update(elapsedTime, m_player.get(), m_particle.get());


	//-------------------------------------------------
	//Debugの更新
	//-------------------------------------------------

	////[2]でデバック切り替え
	//if (input->kbTracker.pressed.D2)
	//{
	//	m_isDebugVisible = !m_isDebugVisible;

	//	//コリジョンの連動して切り替える
	//	if (m_displayCollision)
	//	{
	//		m_displayCollision->SetVisible(m_isDebugVisible);
	//	}
	//}

	DebugUpdate(elapsedTime);

}

//-----------------------------------------------------------------
//描画関連
//-----------------------------------------------------------------
void GamePlayScene::Render()
{
	auto context = GetUserResources()->GetDeviceResources()->GetD3DDeviceContext();
	
	SimpleMath::Matrix view;

	//-------------------------------------------------
	//カメラの取得
	//-------------------------------------------------
	
	if (m_currentCamera == CameraType::Debug)
	{
		view = m_debugCamera->GetCameraMatrix();
	}
	else
	{
		view = m_playerCamera->GetCameraMatrix();
	}

	//-------------------------------------------------
	//シェーダの描画
	//-------------------------------------------------

	m_particle->Render(view, m_proj);

	//-------------------------------------------------
	//スカイドームの描画
	//-------------------------------------------------

	if (m_skyDome)
	{
		m_skyDome->Render(context, view, m_proj);
	}

	//-------------------------------------------------
	//ステージの描画(ギミック関連)
	//-------------------------------------------------

	//ステージ
	if (m_stageManager)
	{
		m_stageManager->Render(context, view, m_proj, nullptr);
	}

	//ギミック
	if (m_gimmickManager)
	{
		m_gimmickManager->Render(context, view, m_proj);
	}

	//魚
	if (m_fishManager)
	{
		m_fishManager->Render(context, view, m_proj, m_shadowRenderer.get());
	}

	//-------------------------------------------------
	//天候の描画
	//-------------------------------------------------
	if (m_weather)
	{
		// 現在のカメラ位置を取得
		// (自身のカメラクラスに合わせて GetEye() や GetPosition() を呼んでください)
		DirectX::SimpleMath::Vector3 camPos = m_playerCamera->GetTargetPosition();

		// 天候を描画
		m_weather->Render(context, m_view, m_proj, camPos);
	}

	//-------------------------------------------------
	//攻撃の描画
	//-------------------------------------------------

	if (m_attackManager)
	{
		m_attackManager->Render(context, view, m_proj);
	}

	//-------------------------------------------------
	//Playerの描画
	//-------------------------------------------------

	if (m_player&& !m_player->IsDead())
	{
		m_player->Render(context, view, m_proj, m_shadowRenderer.get());
	}

	//-------------------------------------------------
	//Enemyの描画
	//-------------------------------------------------

	if (m_enemyManager)
	{
		m_enemyManager->Render(context, view, m_proj, m_shadowRenderer.get());
	}

	//-------------------------------------------------
	//Debugの描画
	//-------------------------------------------------

	if(m_isDebugVisible)
	{
		DebugRender();
	}

	//-------------------------------------------------
	//特定のステージの描画 
	//-------------------------------------------------

	if (m_currentLevel)
	{
		m_currentLevel->Render(GetUserResources());
	}

	//-------------------------------------------------
	//UIの描画 (プレイヤーと敵)
	//-------------------------------------------------

	if (m_gameUI)
	{
		//ボス敵のポインタを取得（EnemyManager経由）
		BossEnemy* boss = nullptr;
		if (m_enemyManager) boss = m_enemyManager->GetBoss();

		m_gameUI->Render(m_player.get(), boss);
	}

	//if (m_isPaused)
	//{
	//	auto debugFont = GetUserResources()->GetDebugFont();
	//	// 画面中央あたりに表示
	//	debugFont->AddString(L"PAUSE", DirectX::SimpleMath::Vector2(600.f, 300.f));
	//}

	// ----------------------------------------------------
	//  ボタンUIテクスチャの描画
	// ----------------------------------------------------

	m_spriteBatch->Begin(SpriteSortMode_Deferred, m_states->NonPremultiplied());


	if (m_textureButtonUI1)
	{
		//ボタンUI
		m_spriteBatch->Draw(m_textureButtonUI1.Get(), SimpleMath::Vector2(0, 0));
	}

	if (m_textureButtonUI2)
	{
		//ボタンUI
		m_spriteBatch->Draw(m_textureButtonUI2.Get(), SimpleMath::Vector2(0, 0));
	}

	m_spriteBatch->End();
}

//-----------------------------------------------------------------
//終了処理関連
//-----------------------------------------------------------------
void GamePlayScene::Finalize()
{
	//BGMのストップ
	AudioManager::GetInstance()->StopBGM();

	m_stageManager.reset();
	m_gimmickManager.reset();

}

//-----------------------------------------------------------------
//モデルやテクスチャ、音読み込み関連
//-----------------------------------------------------------------
void GamePlayScene::CreateDeviceDependentResources()
{
	auto device = GetUserResources()->GetDeviceResources()->GetD3DDevice();
	auto context = GetUserResources()->GetDeviceResources()->GetD3DDeviceContext();
	
	//-------------------------------------------------
	//描画リソースの生成
	//-------------------------------------------------
	
	//スプライトバッチ
	m_spriteBatch = std::make_unique<DirectX::SpriteBatch>(context);

	//ステート
	m_states = std::make_unique<DirectX::CommonStates>(device);

	//影
	m_shadowRenderer = std::make_unique<ShadowRenderer>(device, context);
	
	//当たり判定
	m_displayCollision = std::make_shared<DisplayCollision>(device, context);
	m_displayCollision->SetVisible(false);

	//-------------------------------------------------
	//マネージャーとオブジェクト系の生成
	//-------------------------------------------------
	
	//スカイドーム
	m_skyDome = std::make_unique<SkyDome>(m_deviceResources);
	m_skyDome->Initialize();

	//ステージ
	m_stageManager = std::make_unique<StageManager>(m_deviceResources);

	//ギミック
	m_gimmickManager = std::make_unique<GimmickManager>();

	//攻撃
	m_attackManager = std::make_unique<AttackManager>();
	m_attackManager->SetDisplayCollision(m_displayCollision);
	m_attackManager->SetStates(GetUserResources()->GetCommonStates());

	//弾のモデルの読み込み
	DirectX::EffectFactory fx(device);
	fx.SetDirectory(L"Resources\\Models");
	m_attackManager->SetBulletModel(
		DirectX::Model::CreateFromSDKMESH(device, L"Resources\\Models\\Fish.sdkmesh", fx)
	);

	//敵
	m_enemyManager = std::make_unique<EnemyManager>();

	//UI
	m_gameUI = std::make_unique<GameUI>(m_deviceResources);
	m_gameUI->Initialize();

	//エフェクト
	m_particle = std::make_unique<Particle>();
	m_particle->Initialize(m_deviceResources);

	//-------------------------------------------------
	//テクスチャの読み込み
	//-------------------------------------------------
	
	CreateWICTextureFromFile(device, L"Resources\\Textures\\Try.png", nullptr, m_textureButtonUI1.GetAddressOf());
	CreateWICTextureFromFile(device, L"Resources\\Textures\\Back.png", nullptr, m_textureButtonUI2.GetAddressOf());

	//-------------------------------------------------
	//ステージとステージレベルの構築、音
	//-------------------------------------------------
	
	//BGM音
	AudioManager* audio = AudioManager::GetInstance();
	audio->Initialize();

	//ステージレベル
	switch (NextStageType)
	{
	case StageType::Tutorial:
		m_currentLevel = std::make_unique<LevelTutorial>();
		//晴れ
		m_weather = nullptr;
		//BGM
		audio->LoadSound("Stage1_BGM", L"Resources/Sounds/BGM_Game.wav");
		//音量
		audio->SetBGMVolume(0.2f);

		audio->PlayBGM("Stage1_BGM");
		break;

	case StageType::Stage1:
		m_currentLevel = std::make_unique<LevelStage1>();
		//雨
		m_weather = nullptr;
		//BGM
		audio->LoadSound("Stage1_BGM", L"Resources/Sounds/BGM_Game.wav");

		//音量
		audio->SetBGMVolume(0.2f); audio->PlayBGM("Stage1_BGM");
		break;

	case StageType::Stage2:
		m_currentLevel = std::make_unique<LevelStage2>();

		//天候　雨or雪
		if (rand() % 2 == 0)
		{
			//雨
			m_weather = std::make_unique<Rain>();
		}
		else
		{
			m_weather = std::make_unique<Snow>();
		}

		//BGM
		audio->LoadSound("Stage1_BGM", L"Resources/Sounds/BGM_Game.wav");
		//音量
		audio->SetBGMVolume(0.2f);
		audio->PlayBGM("Stage1_BGM");

		break;
	}

	//天候
	if (m_weather)
	{
		m_weather->Initialize(device);
	}
}

//-----------------------------------------------------------------
//画面サイズ依存のリソース関連
//-----------------------------------------------------------------
void GamePlayScene::CreateWindowSizeDependentResources()
{
	//サイズ取得
	HWND hwnd = m_deviceResources->GetWindow();
	RECT rect = m_deviceResources->GetOutputSize();
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;

	//-------------------------------------------------
	//射影行列を作成
	//-------------------------------------------------

	m_proj = SimpleMath::Matrix::CreatePerspectiveFieldOfView(
		XMConvertToRadians(45.0f),
		static_cast<float>(rect.right) / static_cast<float>(rect.bottom),
		0.1f, 1000.0f
	);

	//-------------------------------------------------
	//カメラの再生成
	//-------------------------------------------------

	//カメラ
	m_debugCamera = std::make_unique<DebugCamera>(width, height);
	//カメラ　主にこっちを使用
	m_playerCamera = std::make_unique<PlayerCamera>(width, height, hwnd);
	//カーソルの非表示
	if (m_currentCamera == CameraType::Player)
	{
		m_playerCamera->HideCursor();
	}

	//-------------------------------------------------
	//各マネージャーの配置の初期化
	//-------------------------------------------------

	//ステージ登録
	m_stageManager->AddStage(L"DefaultStage", hwnd, width, height);
	m_stageManager->SetCurrentStage(L"DefaultStage");

	//ギミックの初期化
	auto wave = std::make_shared<Wave>(m_deviceResources);
	m_gimmickManager->Add(wave);
	m_gimmickManager->Initalize(hwnd, width, height);

	//プレイヤーの生成と初期化
	m_player = std::make_unique<Player>(m_deviceResources, m_displayCollision, m_playerCamera.get());
	m_player->Initalize(hwnd, width, height, m_stageManager->GetCurrentStage());
	m_player->SetAttackManager(m_attackManager.get());

	//魚
	m_fishManager = std::make_unique<FishManager>(m_deviceResources, m_displayCollision, m_stageManager->GetCurrentStage());

	//敵
	m_enemyManager->Initialize(m_deviceResources, m_stageManager->GetCurrentStage(), m_displayCollision);

	//ステージレベルの初期化
	if (m_currentLevel)
	{
		m_currentLevel->Initialize(
			m_deviceResources,
			m_stageManager.get(),
			m_enemyManager.get(),
			m_fishManager.get(),
			m_displayCollision
		);
	}
}

void GamePlayScene::OnDeviceLost()
{
	Finalize();
}

//-----------------------------------------------------------------
//シーン関連
//-----------------------------------------------------------------
void GamePlayScene::SceneChange()
{
	auto input = GetUserResources()->GetInputManager();

	auto transitionMask = GetUserResources()->GetTransitionMask();

	if (m_isChangingScene)
	{
		if (transitionMask->IsClose() && transitionMask->IsEnd())
		{
			ChangeScene<ResultScene>();
		}
		// return; 
	}

	//エンターキーを押すとシーンが変わる
	if (input->kbTracker.pressed.Enter)
	{
		if (NextStageType == StageType::Tutorial)
		{
			ChangeScene<SelectScene>();
			return; // ここで終了
		}
	}

	//強制終了
	if (input->kbTracker.pressed.Enter)
	{

		ChangeScene<ResultScene>();

	}

	//[T]で戻る
	if (input->kbTracker.pressed.T)
	{
		ChangeScene<SelectScene>();
	}

	//[R]で戻る
	if (input->kbTracker.pressed.R)
	{
		ChangeScene<GamePlayScene>();
	}

	//[o]でタイトルへ強制戻る
	if (input->kbTracker.pressed.O)
	{
		ChangeScene<TitleScene>();
	}
}

//-----------------------------------------------------------------
//カメラ関連
//-----------------------------------------------------------------
void GamePlayScene::UpdateCamera()
{
	auto input = GetUserResources()->GetInputManager();

	// [1] でカメラを切り替える
	if (input->kbTracker.pressed.D1)
	{
		if (m_currentCamera == CameraType::Player)
		{
			// 切り替え前 → PlayerCamera の値をコピーする
			m_debugCamera->SetFromOtherCamera(
				m_playerCamera->GetEyePosition(),
				m_playerCamera->GetTargetPosition()
			);
			m_currentCamera = CameraType::Debug;

			m_playerCamera->ShowCursor();
		}
		else
		{
			m_currentCamera = CameraType::Player;
			m_playerCamera->HideCursor();
		}
	}
	//デバッグ
	if (m_currentCamera == CameraType::Debug)
	{
		if (m_debugCamera)
		{
			m_debugCamera->Update();
		}
	}
	else // ゲーム用
	{
		if (m_playerCamera)
		{
			m_playerCamera->Update(
				m_player->GetPosition(),
				GetUserResources()->GetInputManager()->mouse,
				m_player->IsDashing()
			);
		}
	}

}

//-----------------------------------------------------------------
//デバック関連
//-----------------------------------------------------------------
void GamePlayScene::DebugUpdate(float /*timer*/)
{
	//プレイヤーの座標////////////////
	auto pPos = m_player->GetPosition();
	m_playerPos = pPos;

	//敵の座標/////////////////////////
	if (m_enemyManager) {
		// GetBossEnemy() を呼び出して、BossEnemy のインスタンスから座標を取得する
		BossEnemy* boss = m_enemyManager->GetBossEnemy();
		if (boss) {
			m_enemyPos = boss->GetPosition();
		}
		else
		{
			m_enemyPos = Vector3::Zero; // 敵がいない場合のデフォルト値
		}
	}

	//ステージの座標/////////////////////////
	Stage* currentStage = m_stageManager->GetCurrentStage();
	if (currentStage)
	{
		auto sPos = currentStage->GetPosition();
		m_stagePos = sPos;
	}
}

//-----------------------------------------------------------------
//デバック描画関連
//-----------------------------------------------------------------
void GamePlayScene::DebugRender()
{
	// DebugFont を取得
	auto debugFont = GetUserResources()->GetDebugFont();
	const float lineHeight = debugFont->GetFontHeight();
	float y = 150.0f; // 既存のメッセージの下から描画を開始

	
	// プレイヤー情報 
	debugFont->AddString(
		L"--- Player Info ---",
		SimpleMath::Vector2(0.0f, y)
	);
	y += lineHeight;

	// プレイヤー座標 
	{
		std::wstringstream ss;
		ss << L"Player Pos: X=" << std::fixed << std::setprecision(2) << m_playerPos.x
			<< L", Y=" << std::fixed << std::setprecision(2) << m_playerPos.y
			<< L", Z=" << std::fixed << std::setprecision(2) << m_playerPos.z;
		debugFont->AddString(ss.str().c_str(), SimpleMath::Vector2(0.0f, y));
	}
	y += lineHeight;


	if (m_player) {
		// m_player->GetHP() が定義されている前提
		float playerHP = m_player->GetHP();

		// プレイヤー HP
		{
			std::wstringstream ss;
			ss << L"Player HP: " << std::fixed << std::setprecision(0) << playerHP;
			debugFont->AddString(ss.str().c_str(), SimpleMath::Vector2(0.0f, y)); 
		}
		y += lineHeight;
	}

	//敵情報 
	debugFont->AddString(
		L"--- Enemy Info ---",
		SimpleMath::Vector2(0.0f, y)
	);
	y += lineHeight;

	// 敵座標 
	{
		std::wstringstream ss;
		ss << L"Enemy Pos: X=" << std::fixed << std::setprecision(2) << m_enemyPos.x
			<< L", Y=" << std::fixed << std::setprecision(2) << m_enemyPos.y
			<< L", Z=" << std::fixed << std::setprecision(2) << m_enemyPos.z;
		debugFont->AddString(ss.str().c_str(), SimpleMath::Vector2(0.0f, y)); 
	}
	y += lineHeight;

	// 敵のHP
	if (m_enemyManager && m_enemyManager->GetBossEnemy()) {
		float bossHP = m_enemyManager->GetBossEnemy()->GetHP();

		// 敵 HP (C++14対応: wstringstreamで置き換え)
		{
			std::wstringstream ss;
			ss << L"Boss HP: " << std::fixed << std::setprecision(0) << bossHP;
			debugFont->AddString(ss.str().c_str(), SimpleMath::Vector2(0.0f, y));
		}
		y += lineHeight;
	}

	//  ステージ情報 ---
	debugFont->AddString(
		L"--- Stage Info ---",
		SimpleMath::Vector2(0.0f, y)
	);
	y += lineHeight;

	// ステージ座標 
	{
		std::wstringstream ss;
		ss << L"Stage Pos: X=" << std::fixed << std::setprecision(2) << m_stagePos.x
			<< L", Y=" << std::fixed << std::setprecision(2) << m_stagePos.y
			<< L", Z=" << std::fixed << std::setprecision(2) << m_stagePos.z;
		debugFont->AddString(ss.str().c_str(), SimpleMath::Vector2(0.0f, y)); 
	}
	y += lineHeight;

	//スタミナ表示
	float pSut = m_player->GetStamina();

	{
		std::wostringstream ss;
		ss << L"Player Sutamina: "
			<< std::fixed << std::setprecision(0)
			<< pSut;
		// デバッグ文字列として登録
		debugFont->AddString(ss.str().c_str(), SimpleMath::Vector2(0.0f, y));
	}
	y += lineHeight;
}

