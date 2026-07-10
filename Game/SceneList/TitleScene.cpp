
/**
 * @file   TitleScene.h
 * @brief  タイトル画面の初期化・更新・描画を管理するシーンクラス
 * @author 國田知睦
 * @date   2026/07/10
 */

#include "pch.h"
#include <WICTextureLoader.h>
#include "TitleScene.h"

//タイトルシーンー＞選択シーンに移動
#include "SelectScene.h"

#include "Game/Common/ReadData.h"

//-----------------------------------------------------------------
// コンストラクタ
//-----------------------------------------------------------------

TitleScene::TitleScene()
	: 
	m_deviceResources()
{
}

//-----------------------------------------------------------------
// 初期化
//-----------------------------------------------------------------

void TitleScene::Initialize()
{
	//デバイスリソース
	m_deviceResources = GetUserResources()->GetDeviceResources();

	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();

	//カーソルの位置をリセット
	m_currentCursor = MenuType::Start;
	// フェードイン
	GetUserResources()->GetTransitionMask()->Open();
}

//-----------------------------------------------------------------
// 更新
//-----------------------------------------------------------------

void TitleScene::Update(float elapsedTime)
{
	//キーボード用
	auto input = GetUserResources()->GetInputManager();
	
	//波を変更
	if (input->kbTracker.pressed.Tab)
	{
		m_waveManager->ToggleMode();
	}

	//計算しやすいようにintに変換
	//カーソルの位置
	int cursorInt = static_cast<int>(m_currentCursor);
	//カーソルの最大値
	int maxCount = static_cast<int>(MenuType::Count);

	// --- カーソル移動 ---
	if (input->kbTracker.pressed.Up || input->kbTracker.pressed.W)
	{
		//移動音
		AudioManager::GetInstance()->Play("SE_Move", DEFAULT_SE_VOLUME);
		//↑
		cursorInt = (cursorInt - 1 + maxCount) % maxCount;
	}
	if (input->kbTracker.pressed.Down || input->kbTracker.pressed.S)
	{
		//移動音
		AudioManager::GetInstance()->Play("SE_Move",DEFAULT_SE_VOLUME);
		//↓
		cursorInt = (cursorInt + 1) % maxCount;
	}

	//計算したintを戻す
	m_currentCursor = static_cast<MenuType>(cursorInt);

	//フェードアウト
	auto transitionMask = GetUserResources()->GetTransitionMask();
	//フェード
	if (m_isChangingScene)
	{
		if (transitionMask->IsClose() && transitionMask->IsEnd())
		{
			ChangeScene<SelectScene>();
		}
		return; 
	}

	// --- 決定操作 ---
	if (input->kbTracker.pressed.Enter || input->kbTracker.pressed.Space)
	{
		//決定音
		AudioManager::GetInstance()->Play("SE_Click", DEFAULT_SE_CLICK_VOLUME);
        if (m_currentCursor == MenuType::Start)
		{
			//シーンの変更を可
			m_isChangingScene = true;
			transitionMask->Close();
		}
		else
		{
			// やめる -> ゲーム終了
			PostQuitMessage(0);
		}
	}
	
	//背景の更新
	if (m_waveManager)
	{
		m_waveManager->Update(elapsedTime);
	}
	//雪（エフェクト）
	if (m_snow)
	{
		m_snow->Update(elapsedTime);
	}

	//波を揺らしたかったら
	if (m_backgroundStage)
	{
		m_backgroundStage->Update(elapsedTime, m_waveManager.get());
	}
}

//-----------------------------------------------------------------
// 描画
//-----------------------------------------------------------------

void TitleScene::Render()
{
	auto context = GetUserResources()->GetDeviceResources()->GetD3DDeviceContext();
	auto size = GetUserResources()->GetDeviceResources()->GetOutputSize();

	//----------------------------------------------
	//３D背景を描画
	//----------------------------------------------
	//波
	if (m_waveManager)
	{
		m_waveManager->Render(context, m_view, m_proj);
	}
	//ステージ
	if (m_backgroundStage)
	{
		m_backgroundStage->Render(context, m_view, m_proj, nullptr);
	}
	//雪
	if (m_snow)
	{
		// カメラの位置を渡す必要がある
		DirectX::SimpleMath::Vector3 eyePos(0.0f, 5.0f, -15.0f);
		m_snow->Render(context, m_view, m_proj, eyePos);
	}

	float screenW = float(size.right - size.left);
	float screenH = float(size.bottom - size.top);
	float centerX = screenW / 2.0f;

	//2D画像を描画
	m_spriteBatch->Begin(DirectX::SpriteSortMode_Deferred, m_states->NonPremultiplied());
	
	if (m_textureButtonUI)
	{
		//ボタンUI
		m_spriteBatch->Draw(m_textureButtonUI.Get(), DirectX::SimpleMath::Vector2(0, 0));
	}

	//--------------------------------------
	//タイトル
	//--------------------------------------
	if (m_textureTitle)
	{
		// 画像の中心を計算して真ん中に置く
		Microsoft::WRL::ComPtr<ID3D11Resource> res;
		m_textureTitle->GetResource(&res);
		CD3D11_TEXTURE2D_DESC desc;
		Microsoft::WRL::ComPtr<ID3D11Texture2D>(reinterpret_cast<ID3D11Texture2D*>(res.Get()))->GetDesc(&desc);
		DirectX::SimpleMath::Vector2 origin(desc.Width / 2.0f, desc.Height / 2.0f);

		m_spriteBatch->Draw(
			m_textureTitle.Get(),
			DirectX::SimpleMath::Vector2(centerX, screenH * TITLE_POSITION),
			nullptr, DirectX::Colors::White, 0.0f, origin,
			TITLE_SIZE // スケール
		);
	}

	// ボタンを表示する基準の高さ
	float startY = screenH * BUTTON_START_Y_RATIO;
	float stepY = BUTTON_STEP_Y; // ボタンごとの間隔

	//テクスチャを配列にまとめる
	ID3D11ShaderResourceView* buttonTextures[] =
	{
		m_textureStart.Get(),
		m_textureQuit.Get()
	};

	//テクスチャを配列でまとめたものを一か所で描画を行う
	for (int i = 0; i < static_cast<int>(MenuType::Count); i++)
	{
		//現在の回数とカーソルの位置が一致しているか
		bool isSelected = (static_cast<int>(m_currentCursor) == i);
		//選択されていたらサイズを大きく
		float buttonScale = isSelected ? BUTTON_SCALE_SELECTED : BUTTON_SCALE_NORMAL;
		//選択されていたら色を変更
		DirectX::XMVECTOR color = isSelected ? DirectX::Colors::White : DirectX::Colors::Gray;

		DrawTextureCenter(
			buttonTextures[i],
			DirectX::SimpleMath::Vector2(centerX, startY + (stepY * i)),
			buttonScale, color);
	}

	//--------------------------------------
	//カーソルの描画
	//--------------------------------------

	if (m_textureCursor)
	{
		int cursorIndex = static_cast<int>(m_currentCursor);
		//カーソルに応じたY座標を出す
		float cursorY = startY + (stepY * cursorIndex);
		//左側の矢印
		DrawTextureCenter(m_textureCursor.Get(),
			DirectX::SimpleMath::Vector2(centerX - CURSOR_OFFSET_X, cursorY),
			CURSOR_SCALE);
		//右側の矢印 反転
		DrawTextureCenter(m_textureCursor.Get(),
			DirectX::SimpleMath::Vector2(centerX + CURSOR_OFFSET_X, cursorY),
			CURSOR_SCALE, DirectX::Colors::White,
			DirectX::SpriteEffects_FlipHorizontally);
	}
	m_spriteBatch->End();
}

//-----------------------------------------------------------------
// 終了処理
//-----------------------------------------------------------------

void TitleScene::Finalize()
{
}

//-----------------------------------------------------------------
// デバイスに依存するリソースを作成する関数
//-----------------------------------------------------------------

void TitleScene::CreateDeviceDependentResources()
{
	auto device = GetUserResources()->GetDeviceResources()->GetD3DDevice();
	auto context = GetUserResources()->GetDeviceResources()->GetD3DDeviceContext();
	
	//---------------------------------------------------
	//基本リソース
	//---------------------------------------------------
	//スプライトバッチ
	m_spriteBatch = std::make_unique<DirectX::SpriteBatch>(context);
	//ステート
	m_states = std::make_unique<DirectX::CommonStates>(device);

	//---------------------------------------------------
	//テクスチャの読み込み
	//---------------------------------------------------

	DirectX::CreateWICTextureFromFile(device, L"Resources\\Textures\\Title.png",  nullptr, m_textureTitle.GetAddressOf());
	DirectX::CreateWICTextureFromFile(device, L"Resources\\Textures\\Start.png",  nullptr, m_textureStart.GetAddressOf());
	DirectX::CreateWICTextureFromFile(device, L"Resources\\Textures\\Quit.png",   nullptr, m_textureQuit.GetAddressOf());
	DirectX::CreateWICTextureFromFile(device, L"Resources\\Textures\\Cursor.png", nullptr, m_textureCursor.GetAddressOf());

	//---------------------------------------------------
	//演出
	//---------------------------------------------------

	//雪
	m_snow = std::make_unique<Snow>();
	m_snow->Initialize(device);

	//---------------------------------------------------
	//音関連
	//---------------------------------------------------

	AudioManager* audio = AudioManager::GetInstance();
	audio->Initialize();
	audio->LoadSound("Title", L"Resources/Sounds/BGM_Title.wav");
	audio->SetBGMVolume(DEFAULT_BGM_VOLUME);
	audio->PlayBGM("Title");

	//SE 決定音
	audio->LoadSound("SE_Click", L"Resources/Sounds/SE_Click.wav");

	//SE 移動音
	audio->LoadSound("SE_Move", L"Resources/Sounds/SE_MoveCursor.wav");
}

//-----------------------------------------------------------------
// ウインドウサイズに依存するリソースを作成する関数
//-----------------------------------------------------------------

void TitleScene::CreateWindowSizeDependentResources()
{
	HWND hwnd = m_deviceResources->GetWindow();
	RECT size = m_deviceResources->GetOutputSize();
	int width = size.right - size.left;
	int height = size.bottom - size.top;
	float aspectRatio = float(width) / float(height);

	//射影行列
	m_proj = DirectX::SimpleMath::Matrix::CreatePerspectiveFieldOfView(
		DirectX::XMConvertToRadians(CAMERA_FOV),
		aspectRatio,
		CAMERA_NEAR, CAMERA_FAR
	);

	//---------------------------------------
	//３Dカメラの設定（定点）
	//---------------------------------------
	//カメラの位置と注視点
	DirectX::SimpleMath::Vector3 eyePos(0.0f, CAMERA_EYE_Y, CAMERA_EYE_Z);
	DirectX::SimpleMath::Vector3 terget(0.0f, 0.0f, 0.0f);

	//ビュー行列
	m_view = DirectX::SimpleMath::Matrix::CreateLookAt(eyePos, terget, DirectX::SimpleMath::Vector3::Up);

	//---------------------------------------
	//飾り
	//---------------------------------------
	//波
	m_waveManager = std::make_unique<WaveManager>(m_deviceResources);
	m_waveManager->Initialize(hwnd, width, height);

	//ステージ
	m_backgroundStage = std::make_unique<Stage>(GetUserResources()->GetDeviceResources());
	m_backgroundStage->Initialize(
		GetUserResources()->GetDeviceResources()->GetWindow(),
		width, height,
		"Resources\\Stages\\TitleStage.bmp");
}

//-----------------------------------------------------------------
// デバイスロストした時に呼び出される関数
//-----------------------------------------------------------------

void TitleScene::OnDeviceLost()
{
	Finalize();
}

//-----------------------------------------------------------------
// 中心を基準にしてテクスチャを描画する関数
//-----------------------------------------------------------------

void TitleScene::DrawTextureCenter(
	ID3D11ShaderResourceView* texture,
	DirectX::SimpleMath::Vector2 position,
	float scale,
	DirectX::XMVECTOR color,
	DirectX::SpriteEffects effects)
{
	if (!texture)return;

	Microsoft::WRL::ComPtr<ID3D11Resource> res;

	texture->GetResource(&res);
	D3D11_TEXTURE2D_DESC desc;
	((ID3D11Texture2D*)res.Get())->GetDesc(&desc);

	//画像自体の中心点を作る
	DirectX::SimpleMath::Vector2 origin(desc.Width / 2.0f, desc.Height / 2.0f);
	//描画
	m_spriteBatch->Draw(texture, position, nullptr, color, 0.0f, origin, scale, effects);
}