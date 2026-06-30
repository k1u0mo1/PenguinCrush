
/**
 * @file   TitleScene.h
 * @brief  タイトル画面の初期化・更新・描画を管理するシーンクラス
 * @author 國田知睦
 * @date   2026/06/11
 */

#include "pch.h"
#include "TitleScene.h"

//タイトルシーンー＞選択シーンに移動
#include "SelectScene.h"

#include <WICTextureLoader.h>

#include "Game/Common/ReadData.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

//-----------------------------------------------------------------
// コンストラクタ
//-----------------------------------------------------------------

TitleScene::TitleScene()
	: m_deviceResources()
{

}

//-----------------------------------------------------------------
// 初期化
//-----------------------------------------------------------------

void TitleScene::Initialize()
{
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
	
	auto input = GetUserResources()->GetInputManager();

	////エンターキーを押すとシーンが変わる
	//if (input->kbTracker.pressed.Enter)
	//{
	//	ChangeScene<SelectScene>();
	//}
	
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
		AudioManager::GetInstance()->Play("SE_Move");
		//↑
		cursorInt = (cursorInt - 1 + maxCount) % maxCount;
	}
	if (input->kbTracker.pressed.Down || input->kbTracker.pressed.S)
	{
		//移動音
		AudioManager::GetInstance()->Play("SE_Move");
		//↓
		cursorInt = (cursorInt + 1) % maxCount;
	}

	//計算したintを戻す
	m_currentCursor = static_cast<MenuType>(cursorInt);


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
		AudioManager::GetInstance()->Play("SE_Click");

		
        if (m_currentCursor == MenuType::Start)
		{
			// スタート -> セレクト画面へ
			//ChangeScene<SelectScene>();

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
	/*auto debugFont = GetUserResources()->GetDebugFont();*/

	////どのシーンか描画する
	//debugFont->AddString(L"TitleScene", SimpleMath::Vector2(0.0f, debugFont->GetFontHeight()));
	//debugFont->AddString(L"ChangeScene: Enter", SimpleMath::Vector2(0.0f, 60.0f));

	auto context = GetUserResources()->GetDeviceResources()->GetD3DDeviceContext();
	/*auto states = GetUserResources()->GetCommonStates();*/

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
		Vector3 eyePos(0.0f, 5.0f, -15.0f);
		m_snow->Render(context, m_view, m_proj, eyePos);
	}


	float screenW = float(size.right - size.left);
	float screenH = float(size.bottom - size.top);
	float centerX = screenW / 2.0f;

	//2D画像を描画
	m_spriteBatch->Begin(SpriteSortMode_Deferred, m_states->NonPremultiplied());
	
	if (m_textureButtonUI)
	{
		//ボタンUI
		m_spriteBatch->Draw(m_textureButtonUI.Get(), SimpleMath::Vector2(0, 0));
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
		Vector2 origin(desc.Width / 2.0f, desc.Height / 2.0f);

		m_spriteBatch->Draw(
			m_textureTitle.Get(),
			Vector2(centerX, screenH * 0.3f), // 上から30%の位置
			nullptr, Colors::White, 0.0f, origin,
			1.0f // スケール
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
		XMVECTOR color = isSelected ? Colors::White : Colors::Gray;

		DrawTextureCenter(
			buttonTextures[i],
			Vector2(centerX, startY + (stepY * i)),
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
		DrawTextureCenter(m_textureCursor.Get(), SimpleMath::Vector2(centerX - CURSOR_OFFSET_X, cursorY), CURSOR_SCALE);
		//右側の矢印 反転
		DrawTextureCenter(m_textureCursor.Get(), SimpleMath::Vector2(centerX + CURSOR_OFFSET_X, cursorY), CURSOR_SCALE, Colors::White, SpriteEffects_FlipHorizontally);

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
	m_spriteBatch = std::make_unique<SpriteBatch>(context);
	//ステート
	m_states = std::make_unique<CommonStates>(device);

	//---------------------------------------------------
	//テクスチャの読み込み
	//---------------------------------------------------

	CreateWICTextureFromFile(device, L"Resources\\Textures\\Title.png", nullptr, m_textureTitle.GetAddressOf());
	CreateWICTextureFromFile(device, L"Resources\\Textures\\Start.png", nullptr, m_textureStart.GetAddressOf());
	CreateWICTextureFromFile(device, L"Resources\\Textures\\Quit.png", nullptr, m_textureQuit.GetAddressOf());
	CreateWICTextureFromFile(device, L"Resources\\Textures\\Cursor.png", nullptr, m_textureCursor.GetAddressOf());

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
	audio->SetBGMVolume(0.2f);
	audio->PlayBGM("Title");

	//SE 決定音
	audio->SetSEVolume(1.0f);
	audio->LoadSound("SE_Click", L"Resources/Sounds/SE_Click.wav");

	//SE 移動音
	audio->SetSEVolume(0.2f);
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

	//行列
	m_view = SimpleMath::Matrix::CreateLookAt(
		Vector3(0.0f, 5.0f, -10.0f),
		Vector3(0, 0.0f, 0),
		Vector3::Up
	);

	m_proj = SimpleMath::Matrix::CreatePerspectiveFieldOfView(
		XMConvertToRadians(45.0f),
		aspectRatio,
		0.1f, 1000.0f
	);

	//---------------------------------------
	//３Dカメラの設定（定点）
	//---------------------------------------
	//カメラの位置
	Vector3 eyePos(0.0f, 15.0f, -30.0f);
	Vector3 terget(0.0f, 0.0f, 0.0f);

	m_view = SimpleMath::Matrix::CreateLookAt(eyePos, terget, Vector3::Up);


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