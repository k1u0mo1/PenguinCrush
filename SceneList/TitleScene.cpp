
//タイトルシーンクラス

#include "pch.h"
#include "TitleScene.h"

//タイトルシーンー＞選択シーンに移動
#include "SelectScene.h"

#include <WICTextureLoader.h>

#include "Common/ReadData.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

// コンストラクタ
TitleScene::TitleScene()
	: m_deviceResources()
{

}

//初期化
void TitleScene::Initialize()
{
	m_deviceResources = GetUserResources()->GetDeviceResources();

	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();

	//カーソルリセット
	m_currentCursor = 0;

	// フェードイン
	GetUserResources()->GetTransitionMask()->Open();

}

//更新
void TitleScene::Update(float elapsedTime)
{
	elapsedTime;

	auto input = GetUserResources()->GetInputManager();

	////エンターキーを押すとシーンが変わる
	//if (input->kbTracker.pressed.Enter)
	//{
	//	ChangeScene<SelectScene>();
	//}
	
	// --- カーソル移動 ---
	if (input->kbTracker.pressed.Up || input->kbTracker.pressed.W)
	{
		m_currentCursor--;
		if (m_currentCursor < 0) m_currentCursor = 1;
	}
	if (input->kbTracker.pressed.Down || input->kbTracker.pressed.S)
	{
		m_currentCursor++;
		if (m_currentCursor > 1) m_currentCursor = 0;
	}

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

		if (m_currentCursor == 0)
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
	if (m_wave)
	{
		m_wave->Update(elapsedTime);
	}

	if (m_snow)
	{
		m_snow->Update(elapsedTime);
	}

	//波を揺らしたかったら
	if (m_backgroundStage)
	{
		m_backgroundStage->Update(m_wave.get());
	}

}

//描画
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
	if (m_wave)
	{
		m_wave->Render(context, m_view, m_proj);
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

	//--------------------------------------
	//スタート
	//--------------------------------------

	if (m_textureStart)
	{
		Microsoft::WRL::ComPtr<ID3D11Resource> res;

		m_textureStart->GetResource(&res);

		CD3D11_TEXTURE2D_DESC desc;

		Microsoft::WRL::ComPtr<ID3D11Texture2D>(reinterpret_cast<ID3D11Texture2D*>(res.Get()))->GetDesc(&desc);
		Vector2 origin(desc.Width / 2.0f, desc.Height / 2.0f);

		// 選択中なら色を明るく、サイズを大きく
		bool isSelected = (m_currentCursor == 0);
		float scale = isSelected ? 1.2f : 1.0f;
		XMVECTOR color = isSelected ? Colors::White : Colors::Gray; 

		m_spriteBatch->Draw(
			m_textureStart.Get(),
			Vector2(centerX, screenH * 0.6f),
			nullptr,
			color, 
			0.0f,
			origin,
			scale
		);
	}

	//--------------------------------------
	//やめる
	//--------------------------------------

	if (m_textureQuit)
	{
		Microsoft::WRL::ComPtr<ID3D11Resource> res;

		m_textureQuit->GetResource(&res);

		CD3D11_TEXTURE2D_DESC desc;

		Microsoft::WRL::ComPtr<ID3D11Texture2D>(reinterpret_cast<ID3D11Texture2D*>(res.Get()))->GetDesc(&desc);
		Vector2 origin(desc.Width / 2.0f, desc.Height / 2.0f);

		bool isSelected = (m_currentCursor == 1);
		float scale = isSelected ? 1.2f : 1.0f;
		XMVECTOR color = isSelected ? Colors::White : Colors::Gray; 

		m_spriteBatch->Draw(
			m_textureQuit.Get(),
			Vector2(centerX, screenH * 0.75f),
			nullptr,
			color, 
			0.0f,
			origin, 
			scale
		);
	}

	//--------------------------------------
	//カーソルの描画
	//--------------------------------------

	if (m_textureCursor)
	{
		//画像サイズの取得
		Microsoft::WRL::ComPtr<ID3D11Resource> res;
		m_textureCursor->GetResource(&res);
		CD3D11_TEXTURE2D_DESC desc;

		Microsoft::WRL::ComPtr<ID3D11Texture2D>(reinterpret_cast<ID3D11Texture2D*>(res.Get()))->GetDesc(&desc);
		Vector2 origin(desc.Width / 2.0f, desc.Height / 2.0f);

		//カーソルの位置計算
		SimpleMath::Vector2 cursorPos;

		
		//Startを選択中
		if (m_currentCursor == 0)
		{
			cursorPos.y = screenH * 0.6f;
		}
		//Quitを選択中
		else
		{
			cursorPos.y = screenH * 0.75f;
		}
		cursorPos.x = centerX-120.0f;

		//カーソルの描画
		m_spriteBatch->Draw(
			m_textureCursor.Get(),
			cursorPos,
			nullptr,
			Colors::White,
			0.0f,
			origin,
			0.25f
		);

		cursorPos.x = centerX + 120.0f;

		//カーソルの描画
		m_spriteBatch->Draw(
			m_textureCursor.Get(),
			cursorPos,
			nullptr,
			Colors::White,
			0.0f,
			origin,
			0.25f,
			SpriteEffects_FlipHorizontally //反転
		);

	}

	m_spriteBatch->End();

}

//終了処理
void TitleScene::Finalize()
{

}

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
	audio->LoadSound("Title", L"Resources/Sounds/あわ雪村.wav");
	audio->SetBGMVolume(0.2f);
	audio->PlayBGM("Title");

	//SE 決定音
	audio->SetSEVolume(1.0f);
	audio->LoadSound("SE_Click", L"Resources/Sounds/決定ボタンを押す44.wav");

}

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
	m_wave = std::make_unique<Wave>(m_deviceResources);
	m_wave->Initialize(hwnd, width, height);

	//ステージ
	m_backgroundStage = std::make_unique<Stage>(GetUserResources()->GetDeviceResources());
	m_backgroundStage->Initialize(GetUserResources()->GetDeviceResources()->GetWindow(), width, height);

}

void TitleScene::OnDeviceLost()
{
	Finalize();
}
