
/**
 * @file   GamePlayScene.cpp
 * @brief  ゲーム画面の初期化・更新・描画を管理するクラス
 * @author 國田知睦
 * @date   2026/06/29
 */

#include "pch.h"
#include <sstream> 
#include <iomanip>

#include "GamePlayScene.h"

//ゲームプレイシーンー＞リザルトシーンに移動
#include "ResultScene.h"
#include "SelectScene.h"
#include "TitleScene.h"

//どのステージ
#include "Game/SceneList/StageLevel/LevelBase.h"
#include "Game/SceneList/StageLevel/LevelTutorial.h"
#include "Game/SceneList/StageLevel/LevelEasy.h"
#include "Game/SceneList/StageLevel/LevelNormal.h"
#include "Game/SceneList/StageLevel/LevelHard.h"

#include "Game/WeatherList/Rain.h"
#include "Game/WeatherList/Snow.h"
#include "Game/SoundList/AudioManager.h"
#include "Game/PlayerList/AttackList/AttackManager.h"
#include "Game/Common/ObjectCharacter/HitStop.h"
#include "Game/Common/ObjectCharacter/ModelManager.h"

GamePlayScene::StageType GamePlayScene::NextStageType =
GamePlayScene::StageType::EASY;

//-----------------------------------------------------------------
// コンストラクタ
//-----------------------------------------------------------------

GamePlayScene::GamePlayScene()
	: 
	m_deviceResources{},
	m_isFinished(false),
	m_isStageClear(false),
	m_finishTimer(FINISH_WAIT_TIME),
	m_isDebugVisible(false),
	m_isPaused(false),
	m_hitStop()
{
}

//-----------------------------------------------------------------
//	デストラクタ
//-----------------------------------------------------------------

GamePlayScene::~GamePlayScene()
{
}

//-----------------------------------------------------------------
// 初期化
//-----------------------------------------------------------------

void GamePlayScene::Initialize()
{
	//デバイスリソースを取得
	m_deviceResources = GetUserResources()->GetDeviceResources();
	//デバイスに依存するリソース呼ぶ
	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
	
	//終わったか
	m_isFinished = false;
	//クリアしたか
	m_isStageClear = false;
	//デバックを表示・非表示
	m_isDebugVisible = false;
	//現在、一時停止中か
	m_isPaused = false;
	//最初のオープニング中か
	m_isOpening = true;
	//シーン移動中
	m_isChangingScene = false;

	//演出の時間
	m_openingTimer = 0.0f;
	//終わったときに何秒後に
	m_finishTimer = FINISH_WAIT_TIME;

	//オープニング演出のセットアップ
	auto boss = m_enemyManager->GetBossEnemy();
	if (boss != nullptr)
	{
		boss->SetState(EnemyState::Loading, OPENING_DURATION);
	}
	//フェードイン開始
	GetUserResources()->GetTransitionMask()->Open();

	//音量設定 
	AudioManager::GetInstance()->SetSEVolume(DEFAULT_SE_VOLUME);
}

//-----------------------------------------------------------------
// 更新
//-----------------------------------------------------------------

void GamePlayScene::Update(float elapsedTime)
{
	//ボタン
	auto input = GetUserResources()->GetInputManager();
	//フェード
	auto transitionMask = GetUserResources()->GetTransitionMask();

	//BGMの更新
	AudioManager::GetInstance()->Update();

	//ヒットストップの更新
	m_hitStop->HitStopUpdate(elapsedTime);
	//ヒットストップが有効なときは時間を止める
	float hitStopFactor = 
		m_hitStop->IsHitStopActive() ? 
		HIT_STOP_TIME_SCALE : NORMAL_TIME_SCALE;

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
	//シーン系関連
	SceneChange();
	//-------------------------------------------------
	//敵登場の更新
	//-------------------------------------------------
	if (m_isOpening)
	{
		//ボス敵の出現タイマーの加算
		m_openingTimer += elapsedTime;

		if (m_openingTimer >= OPENING_DURATION)
		{
			//オープニング終了
			m_isOpening = false;

			//敵が存在するかチェックする 
			auto boss = m_enemyManager->GetBossEnemy();
			if (boss != nullptr)
			{
				//ボスを通常状態へ
				boss->SetState(EnemyState::Idle, 0.0f);

				//タイマーをリセット
				m_openingTimer = 0.0f;
			}
		}
	}

	//ステージの情報を渡す
	if (m_currentLevel)
	{
		//ステージ固有の毎フレームの更新処理
		m_currentLevel->Update(elapsedTime, GetUserResources());
	}

	//-------------------------------------------------
	//勝敗の更新
	//-------------------------------------------------
	if (m_isFinished)
	{
		//勝敗がついた後のシーンタイマーの減算
		m_finishTimer -= elapsedTime;
		if (m_finishTimer <= 0.0f)
		{
			//リザルトシーンへ
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
		//シーン切り替えON
		m_isChangingScene = true;
		//フェード演出
		transitionMask->Close();
		return;
	}

	//-------------------------------------------------
	//敵の全滅orボスのクリア判定
	//-------------------------------------------------
	if (m_enemyManager)
	{
		//ボスが存在しているステージ
		if (m_enemyManager->HasBoss())
		{
			//ボスを渡す
			BossEnemy* boss = m_enemyManager->GetBoss();
			if (boss && boss->IsDead())
			{
				//終了フラグ（プレイヤー勝ち）
				m_isStageClear = true;
			}
		}
		else
		{
			//生きている敵が０体か
			if (!m_isOpening && m_enemyManager->GetActiveEnemies().empty())
			{
				//終了フラグ（プレイヤー勝ち）
				m_isStageClear = true;
			}
		}

		//クリア条件を満たしているか
		if (m_isStageClear)
		{
			//選択画面のクリアフラグを立てる
			SelectScene::s_isClearedList[(int)NextStageType] = true;
			//終了フラグ
			m_isFinished = true;
			//「勝ち」
			GetUserResources()->SetGameClear(true);
			//シーン切り替えON
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
		//全てのギミックの状態を更新
		m_gimmickManager->Update(elapsedTime);
	}

	//-------------------------------------------------
	//ステージ・波 更新（波）
	//-------------------------------------------------
	//波の更新
	m_waveManager->Update(elapsedTime);

	if (m_stageManager)
	{
		//ステージの更新
		m_stageManager->Update(elapsedTime, m_waveManager.get());
	}
	//-------------------------------------------------
	//パーティクルの更新
	//-------------------------------------------------
	if (m_particle)
	{
		//パーティクルの移動・寿命計算などの更新
		m_particle->Update(elapsedTime);
	}

	//-------------------------------------------------
	//プレイヤーの更新
	//-------------------------------------------------
	if (m_player && !m_player->IsDead())
	{
		m_player->Update(
			elapsedTime * hitStopFactor,
			input->mouse,
			input->mouseTracker,
			m_stageManager->GetCurrentStage(),
			m_waveManager.get(),
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
		//魚のスポーンタイマーの更新とプレイヤーとの当たり判定
		m_fishManager->Update(elapsedTime, m_player.get());
	}

	//-------------------------------------------------
	//AttackManagerの更新
	//-------------------------------------------------
	if (m_attackManager)
	{
		//現在生きているすべての敵のリストを取得用
		std::vector<EnemyBase*> activeEnemies = m_enemyManager->GetActiveEnemies();
		//攻撃の更新処理と敵の当たり判定
		m_attackManager->Update(elapsedTime, activeEnemies, m_particle.get(), m_hitStop.get());
	}

	//-------------------------------------------------
	//敵の更新
	//-------------------------------------------------
	m_enemyManager->Update(
		elapsedTime * hitStopFactor,
		m_player.get(),
		m_particle.get());

	//-------------------------------------------------
	//プレイヤーと敵の座標を流氷を渡す処理
	//-------------------------------------------------
	if (m_stageManager)
	{
		//現在のステージクラスを取得
		Stage* currentStage = m_stageManager->GetCurrentStage();
		if (currentStage)
		{
			//プレイヤーが生きている時、足元の流氷のHPを削る
			if (m_player && !m_player->IsDead())
			{
				//プレイヤーの座標を取得
				DirectX::SimpleMath::Vector3 playerPos = m_player->GetPosition();
				//流氷の個別足場とプレイヤーの判定を調べる
				currentStage->ApplyDamegeToFloe(elapsedTime, playerPos.x, playerPos.z);
			}

			//生きている敵のリストを取得
			if (m_enemyManager)
			{
				//現在生きているすべての敵のリストを取得
				std::vector<EnemyBase*> activeEnemies = m_enemyManager->GetActiveEnemies();
				for (EnemyBase* enemy : activeEnemies)
				{
					//敵の座標を取得
					DirectX::SimpleMath::Vector3 enemyPos = enemy->GetPosition();
					//流氷の個別足場と敵の判定を調べる
					currentStage->ApplyDamegeToFloe(elapsedTime, enemyPos.x, enemyPos.z);
				}
			}
		}
	}
}

//-----------------------------------------------------------------
// 描画
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
		//デバッグカメラ
		view = m_debugCamera->GetCameraMatrix();
	}
	else
	{
		//プレイヤーカメラ（ゲームプレイヤーカメラ）
		view = m_playerCamera->GetCameraMatrix();
	}

	//-------------------------------------------------
	//シェーダの描画
	//-------------------------------------------------
	//3D空間へのパーティクル描画処理
	m_particle->Render(view, m_proj);

	//-------------------------------------------------
	//スカイドームの描画
	//-------------------------------------------------
	if (m_skyDome)
	{
		//スカイドームモデルの描画
		m_skyDome->Render(context, view, m_proj);
	}

	//-------------------------------------------------
	//波の描画
	//-------------------------------------------------
	if (m_waveManager)
	{
		m_waveManager->Render(context, view, m_proj);
	}

	//-------------------------------------------------
	//ステージの描画
	//-------------------------------------------------
	if (m_stageManager)
	{
		m_stageManager->Render(context, view, m_proj, nullptr);
	}
	//-------------------------------------------------
	//ギミックの描画
	//-------------------------------------------------
	if (m_gimmickManager)
	{
		m_gimmickManager->Render(context, view, m_proj);
	}
	//-------------------------------------------------
	//魚の描画
	//-------------------------------------------------
	if (m_fishManager)
	{
		m_fishManager->Render(context, view, m_proj, m_shadowRenderer.get());
	}
	//-------------------------------------------------
	//プレイヤーの攻撃の描画
	//-------------------------------------------------
	if (m_attackManager)
	{
		m_attackManager->Render(context, view, m_proj);
	}
	//-------------------------------------------------
	//プレイヤーの描画　モデル、影、当たり判定の描画
	//-------------------------------------------------
	if (m_player&& !m_player->IsDead())
	{
		m_player->Render(context, view, m_proj, m_shadowRenderer.get());
	}
	//-------------------------------------------------
	//敵の描画　敵達と敵の攻撃の描画
	//-------------------------------------------------
	if (m_enemyManager)
	{
		m_enemyManager->Render(context, view, m_proj, m_shadowRenderer.get());
	}
	//-------------------------------------------------
	//特定のステージの描画 
	//-------------------------------------------------
	if (m_currentLevel)
	{
		//現在のカメラの座標を取得
		DirectX::SimpleMath::Vector3 camPos = m_playerCamera->GetTargetPosition();
		//ステージの描画
		m_currentLevel->Render(context, view, m_proj, camPos);
	}
	//-------------------------------------------------
	//UIの描画 (プレイヤーと敵)
	//-------------------------------------------------
	if (m_gameUI)
	{
		//ボス敵のポインタを取得（EnemyManager経由）
		BossEnemy* boss = nullptr;
		//現在のボスのオブジェクトを取得
		if (m_enemyManager) boss = m_enemyManager->GetBoss();
		//パラメータ系のUI描画
		m_gameUI->Render(m_player.get(), boss);
	}
	// ----------------------------------------------------
	//  ボタンUIテクスチャの描画
	// ----------------------------------------------------
	m_spriteBatch->Begin(SpriteSortMode_Deferred, m_states->NonPremultiplied());

	if (m_textureButtonUI1)
	{
		//ボタンUI1
		m_spriteBatch->Draw(m_textureButtonUI1.Get(), SimpleMath::Vector2(0, 0));
	}
	if (m_textureButtonUI2)
	{
		//ボタンUI2
		m_spriteBatch->Draw(m_textureButtonUI2.Get(), SimpleMath::Vector2(0, 0));
	}

	m_spriteBatch->End();
}

//-----------------------------------------------------------------
// 終了処理
//-----------------------------------------------------------------

void GamePlayScene::Finalize()
{
	//BGMのストップ
	AudioManager::GetInstance()->StopBGM();
	//ステージマネージャーをリセット
	m_stageManager.reset();
	//ギミックマネージャーをリセット
	m_gimmickManager.reset();
}

//-----------------------------------------------------------------
// デバイスに依存するリソースを作成する関数 モデルやテクスチャ、音読み込み関連
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
	//表示切り替え用の関数
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
	//表示用のコリジョンを設定
	m_attackManager->SetDisplayCollision(m_displayCollision);
	//描画ステートを設定
	m_attackManager->SetStates(GetUserResources()->GetCommonStates());
	//ヒットストップ
	m_hitStop = std::make_unique<HitStop>();
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
	//ステージとステージレベルの構築
	//-------------------------------------------------
	//ステージレベル
	switch (NextStageType)
	{
		//チュートリアル
	case StageType::Tutorial:
		m_currentLevel = std::make_unique<LevelTutorial>();
		break;

		//イージー
	case StageType::EASY:
		m_currentLevel = std::make_unique<LevelEasy>();
		break;

		//ノーマル
	case StageType::NORMAL:
		m_currentLevel = std::make_unique<LevelNormal>();
		break;

		//ハード
	case StageType::HARD:
		m_currentLevel = std::make_unique<LevelHard>();
		break;
	}
}

//-----------------------------------------------------------------
// ウインドウサイズに依存するリソースを作成する関数
//-----------------------------------------------------------------

void GamePlayScene::CreateWindowSizeDependentResources()
{
	auto device = m_deviceResources->GetD3DDevice();

	//サイズ取得
	HWND hwnd = m_deviceResources->GetWindow();
	RECT rect = m_deviceResources->GetOutputSize();
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;
	//-------------------------------------------------
	//射影行列を作成
	//-------------------------------------------------
	m_proj = SimpleMath::Matrix::CreatePerspectiveFieldOfView(
		XMConvertToRadians(CAMERA_FOV),
		static_cast<float>(rect.right) / static_cast<float>(rect.bottom),
		CAMERA_NEAR_CLIP, CAMERA_FAR_CLIP
	);
	//-------------------------------------------------
	//カメラの再生成
	//-------------------------------------------------
	//デバッグカメラ
	m_debugCamera = std::make_unique<DebugCamera>(width, height);
	//ゲームプレイヤーカメラ　主にこっちを使用
	m_playerCamera = std::make_unique<PlayerCamera>(width, height, hwnd);

	//カーソルの非表示
	if (m_currentCamera == CameraType::Player)
	{
		m_playerCamera->HideCursor();
	}

	//-------------------------------------------------
	//各マネージャーの配置の初期化
	//-------------------------------------------------
	//波マネージャーの初期化
	m_waveManager = std::make_unique<WaveManager>(m_deviceResources);
	m_waveManager->Initialize(hwnd, width, height);

	//モデルを一括で管理するシングルトンインスタンスを取得
	ModelManager::GetInstance()->Initialize(device);

	//レベル初期化後に安全になった後に現在のステージポインタを取得
	auto currentStage = m_stageManager->GetCurrentStage();

	//魚マネージャーの初期化
	m_fishManager = std::make_unique<FishManager>(m_deviceResources, m_displayCollision, currentStage);
	//敵 一旦初期化
	m_enemyManager->Initialize(m_deviceResources, nullptr, m_displayCollision);

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

	//レベル初期化が確定したら現在の最新ステージポインタを再設定
	currentStage = m_stageManager->GetCurrentStage();

	//魚マネージャーの初期化
	m_fishManager = std::make_unique<FishManager>(m_deviceResources, m_displayCollision, currentStage);
	//敵マネージャーの初期化
	m_enemyManager->Initialize(m_deviceResources, currentStage, m_displayCollision);
	//プレイヤーの生成と初期化
	m_player = std::make_unique<Player>(
		m_deviceResources, m_displayCollision, m_playerCamera.get());
	m_player->Initialize(
		hwnd, width, height, currentStage);
	//プレイヤーのどの攻撃かをもらうか判別
	m_player->SetAttackManager(m_attackManager.get());
}

//-----------------------------------------------------------------
// デバイスロストした時に呼び出される関数
//-----------------------------------------------------------------

void GamePlayScene::OnDeviceLost()
{
	//終了処理
	Finalize();
}

//-----------------------------------------------------------------
// シーン系関連
//-----------------------------------------------------------------

void GamePlayScene::SceneChange()
{
	auto input = GetUserResources()->GetInputManager();
	auto transitionMask = GetUserResources()->GetTransitionMask();

	//フェードアウト
	if (m_isChangingScene)
	{
		if (transitionMask->IsClose() && transitionMask->IsEnd())
		{
			//シーンを切り替える
			ChangeScene<ResultScene>();
		} 
	}

	//[Enter]強制終了 クリア判定
	if (input->IsSubmitAction())
	{
		//各ステージのクリアフラグ配列
		SelectScene::s_isClearedList[(int)NextStageType] = true;
		//ステージクリアをON
		m_isStageClear = true;
		//シーンの切り替え 
		ChangeScene<ResultScene>();
	}
	//[T]で戻る
	if (input->IsCancelAction())
	{
		ChangeScene<SelectScene>();
	}
	//[R]で戻る
	if (input->IsRetryAction())
	{
		ChangeScene<GamePlayScene>();
	}
	//[O]でタイトルへ強制戻る
	if (input->IsForceQuitAction())
	{
		ChangeScene<TitleScene>();
	}
}

//-----------------------------------------------------------------
// カメラの更新
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
			//カメラをデバッグへ
			m_currentCamera = CameraType::Debug;
			m_playerCamera->ShowCursor();
		}
		else
		{
			//プレイヤーカメラ
			m_currentCamera = CameraType::Player;
			m_playerCamera->HideCursor();
		}
	}
	//デバッグ
	if (m_currentCamera == CameraType::Debug)
	{
		if (m_debugCamera)
		{
			//デバッグカメラの更新
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
