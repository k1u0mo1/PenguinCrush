
//リザルトシーンクラス
//
#include "pch.h"
#include "ResultScene.h"

//リザルトシーン->タイトルシーンに移動

#include "TitleScene.h"
#include "GamePlayScene.h"
#include "SelectScene.h"

//BGM
#include <SoundList/AudioManager.h>

bool ResultScene::isClear = false;

using namespace DirectX;

//-----------------------------------------------------------------
//コンストラクタ
//-----------------------------------------------------------------
ResultScene::ResultScene()
	: m_deviceResources{}
	, m_currentCursor(0)
{

}

//-----------------------------------------------------------------
//初期化関連
//-----------------------------------------------------------------
void ResultScene::Initialize()
{
	m_deviceResources = GetUserResources()->GetDeviceResources();

	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();

	//選択番号
	m_currentCursor = 0;
	//シーンの移動
	m_isChangingScene = false;

	m_animationTimer = 0.0f;

	//フェードイン
	GetUserResources()->GetTransitionMask()->Open();
	

	////---------------------------------------------------
	////勝敗でBGMの違う再生
	////---------------------------------------------------
	//AudioManager* audio = AudioManager::GetInstance();
	//audio->Initialize();

	////勝ち
	//if (isClear)
	//{
	//	// 勝った時のBGM 
	//	audio->LoadSound("Result_Clear", L"Resources/Sounds/Light-Steps.wav");
	//	//音量
	//	audio->SetBGMVolume(0.2f);
	//	audio->PlayBGM("Result_Clear");
	//}
	////負け
	//else
	//{
	//	// 負けた時のBGM
	//	audio->LoadSound("Result_Over", L"Resources/Sounds/Tristesse.wav");
	//	//音量
	//	audio->SetBGMVolume(0.2f);
	//	audio->PlayBGM("Result_Over");
	//}


	////SE 決定音
	//audio->SetSEVolume(1.0f);
	//audio->LoadSound("SE_Click", L"Resources/Sounds/決定ボタンを押す44.wav");

}

//-----------------------------------------------------------------
//更新処理関連
//-----------------------------------------------------------------
void ResultScene::Update(float elapsedTime)
{
	elapsedTime;


	auto input = GetUserResources()->GetInputManager();

	//エンターキーを押すとシーンが変わる
	/*if (input->kbTracker.pressed.Enter|| input->kbTracker.pressed.Space)
	{
		ChangeScene<TitleScene>();
	}*/

	//[o]でタイトルへ強制戻る
	if (input->kbTracker.pressed.O)
	{
		ChangeScene<TitleScene>();
	}

	//タイマー
	m_animationTimer += elapsedTime;

	// 天候の更新
	if (m_snow)
	{
		m_snow->Update(elapsedTime);
	}

	if (m_rain)
	{
		m_rain->Update(elapsedTime);
	}

	// --- カーソル移動 ---
	if (input->kbTracker.pressed.Up || input->kbTracker.pressed.W)
	{
		m_currentCursor--;

		if (m_currentCursor < 0)
			m_currentCursor = MENU_COUNT - 1; // 下からループ
	}

	if (input->kbTracker.pressed.Down || input->kbTracker.pressed.S)
	{
		m_currentCursor++;

		if (m_currentCursor >= MENU_COUNT)
			m_currentCursor = 0; // 上からループ
	}

	/*auto transitionMask = GetUserResources()->GetTransitionMask();*/

	//---------------------------------------------------
	//どのシーンに移動するか
	//---------------------------------------------------

	// 決定操作
	if (input->kbTracker.pressed.Enter || input->kbTracker.pressed.Space)
	{
		//決定音
		AudioManager::GetInstance()->Play("SE_Click");

		switch (m_currentCursor)
		{
			//リトライ
		case 0:
			ChangeScene<GamePlayScene>();
			break;

			//選択画面へ
		case 1:
			ChangeScene<SelectScene>();
			break;

			//タイトルへ
		case 2:
			ChangeScene<TitleScene>();
			break;
		}
	}

}

//-----------------------------------------------------------------
//描画処理関連
//-----------------------------------------------------------------
void ResultScene::Render()
{

	////どのシーンか描画する
	//debugFont->AddString(L"ResultScene", SimpleMath::Vector2(0.0f, debugFont->GetFontHeight()));
	//debugFont->AddString(L"ChangeScene: Enter", SimpleMath::Vector2(0.0f, 60.0f));

	auto context = GetUserResources()->GetDeviceResources()->GetD3DDeviceContext();
	/*auto states = GetUserResources()->GetCommonStates();*/

	//-----------------------------------------
	// モデルの動き
	//-----------------------------------------

	SimpleMath::Vector3 scale(1.0f, 1.0f, 1.0f);   //サイズ
	SimpleMath::Vector3 rotation(0.0f, 0.0f, 0.0f);//回転
	SimpleMath::Vector3 position(0.0f, 0.0f, 0.0f);//座標

	//勝った時
	if (isClear)
	{
		float jumpSpeed = 10.0f;//速さ
		float jnmpHeight =1.0f;//高さ

		position.y = abs(sinf(m_animationTimer * jumpSpeed)) * jnmpHeight;

		//回る
		rotation.y = m_animationTimer * 2.0f;
	}
	//負けたとき
	else
	{
		rotation.x = -DirectX::XM_PIDIV2;//-90度
		rotation.y = DirectX::XM_PIDIV2; //90度
		
		//rotation.z = DirectX::XM_PIDIV4;//-90度

		position.y = 0.5f;
	}

	//-----------------------------------------
	// 行列で描画
	//-----------------------------------------

	if (m_resultModel)
	{
		SimpleMath::Matrix world =
			SimpleMath::Matrix::CreateScale(scale) *
			SimpleMath::Matrix::CreateRotationX(rotation.x) *
			SimpleMath::Matrix::CreateRotationY(rotation.y) *
			SimpleMath::Matrix::CreateRotationZ(rotation.z) *
			SimpleMath::Matrix::CreateTranslation(position);

		m_resultModel->Draw(context, *m_states, world, m_view, m_proj);
	}


	//-----------------------------------------
	// テクスチャ
	//-----------------------------------------
	//画面サイズを取得
	auto size = GetUserResources()->GetDeviceResources()->GetOutputSize();
	float screenW = float(size.right - size.left);
	float screenH = float(size.bottom - size.top);
	float centerX = screenW / 2.0f;

	m_spriteBatch->Begin(SpriteSortMode_Deferred, m_states->NonPremultiplied());

	//勝敗　勝ち
	if (isClear)
	{
		if (m_textureClear)
		{
			// 真ん中に表示するための計算
			Microsoft::WRL::ComPtr<ID3D11Resource> res;
			m_textureClear->GetResource(&res);
			CD3D11_TEXTURE2D_DESC desc;
			Microsoft::WRL::ComPtr<ID3D11Texture2D>(reinterpret_cast<ID3D11Texture2D*>(res.Get()))->GetDesc(&desc);
			DirectX::SimpleMath::Vector2 origin(desc.Width / 2.0f, desc.Height / 2.0f);

			// 画面の上の方(30%)に表示
			m_spriteBatch->Draw(
				m_textureClear.Get(),
				DirectX::SimpleMath::Vector2(centerX, screenH * 0.3f),
				nullptr, Colors::White, 0.0f, origin,
				2.0f // サイズ調整が必要ならここを変える
			);

			//雪
			if (m_snow)
			{
				// カメラの位置を渡す必要がある
				Vector3 eyePos(0.0f, 5.0f, -15.0f);
				m_snow->Render(context, m_view, m_proj, eyePos);
			}
		}
	}
	//負け
	else
	{
		if (m_textureOver)
		{
			Microsoft::WRL::ComPtr<ID3D11Resource> res;
			m_textureOver->GetResource(&res);
			CD3D11_TEXTURE2D_DESC desc;
			Microsoft::WRL::ComPtr<ID3D11Texture2D>(reinterpret_cast<ID3D11Texture2D*>(res.Get()))->GetDesc(&desc);
			DirectX::SimpleMath::Vector2 origin(desc.Width / 2.0f, desc.Height / 2.0f);

			m_spriteBatch->Draw(
				m_textureOver.Get(),
				DirectX::SimpleMath::Vector2(centerX, screenH * 0.3f),
				nullptr,
				Colors::White,
				0.0f, 
				origin,
				2.0f
			);

			//雨
			if (m_snow)
			{
				// カメラの位置を渡す必要がある
				Vector3 eyePos(0.0f, 5.0f, -15.0f);
				m_rain->Render(context, m_view, m_proj, eyePos);
			}
		}
	}


	// ----------------------------------------------------
	//  ボタンUIテクスチャの描画
	// ----------------------------------------------------

	if (m_textureButtonUI)
	{
		//ボタンUI
		m_spriteBatch->Draw(m_textureButtonUI.Get(), SimpleMath::Vector2(0, 0));
	}

	// ボタンを表示する基準の高さ
	float startY = screenH * 0.6f;
	float stepY = 100.0f; // ボタンごとの間隔

	//  Retryボタンの描画 
	if (m_textureRetry) 
	{
		// 0番目が選ばれていたら大きく
		float retryScale = (m_currentCursor == 0) ? 1.2f : 1.0f;
		// 選ばれていない時は少し暗くする
		XMVECTOR color = (m_currentCursor == 0) ? Colors::White : Colors::Gray;

		// 画像の中心を取得
		Microsoft::WRL::ComPtr<ID3D11Resource> res;
		m_textureRetry->GetResource(&res);
		CD3D11_TEXTURE2D_DESC desc;
		((ID3D11Texture2D*)res.Get())->GetDesc(&desc);
		Vector2 origin(desc.Width / 2.0f, desc.Height / 2.0f);

		m_spriteBatch->Draw(m_textureRetry.Get(), Vector2(centerX, startY), nullptr, color, 0.f, origin, retryScale);
	}

	// Selectボタンの描画
	if (m_textureSelect)
	{
		float selectScale = (m_currentCursor == 1) ? 1.2f : 1.0f;
		XMVECTOR color = (m_currentCursor == 1) ? Colors::White : Colors::Gray;

		Microsoft::WRL::ComPtr<ID3D11Resource> res;
		m_textureSelect->GetResource(&res);
		CD3D11_TEXTURE2D_DESC desc;
		((ID3D11Texture2D*)res.Get())->GetDesc(&desc);
		Vector2 origin(desc.Width / 2.0f, desc.Height / 2.0f);

		m_spriteBatch->Draw(m_textureSelect.Get(), Vector2(centerX, startY + stepY), nullptr, color, 0.f, origin, selectScale);
	}

	// Titleボタンの描画
	if (m_textureTitle)
	{
		float titleScale = (m_currentCursor == 2) ? 1.2f : 1.0f;
		XMVECTOR color = (m_currentCursor == 2) ? Colors::White : Colors::Gray;

		Microsoft::WRL::ComPtr<ID3D11Resource> res;
		m_textureTitle->GetResource(&res);
		CD3D11_TEXTURE2D_DESC desc;
		((ID3D11Texture2D*)res.Get())->GetDesc(&desc);
		Vector2 origin(desc.Width / 2.0f, desc.Height / 2.0f);

		m_spriteBatch->Draw(m_textureTitle.Get(), Vector2(centerX, startY + stepY * 2), nullptr, color, 0.f, origin, titleScale);
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


		//Retryを選択中
		if (m_currentCursor == 0)
		{
			cursorPos.y = screenH * 0.60f;
		}
		//Selectを選択中
		else if (m_currentCursor == 1)
		{
			cursorPos.y = screenH * 0.75f;
		}
		//Titleを選択中
		else
		{
			cursorPos.y = screenH * 0.90f;
		}
		cursorPos.x = centerX - 150.0f;

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

		cursorPos.x = centerX + 150.0f;

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

	// ガイド表示
	//debugFont->AddString(L"[Z] Decision  [UP/DOWN] Select", Vector2(50, screenH - 50), Colors::Yellow);

	m_spriteBatch->End();

}

//-----------------------------------------------------------------
//終了処理関連
//-----------------------------------------------------------------
void ResultScene::Finalize()
{

}

//-----------------------------------------------------------------
//関連
//-----------------------------------------------------------------
void ResultScene::CreateDeviceDependentResources()
{
	auto device = GetUserResources()->GetDeviceResources()->GetD3DDevice();
	auto context = GetUserResources()->GetDeviceResources()->GetD3DDeviceContext();

	//---------------------------------------------------
	//共通ステートとバッチ
	//---------------------------------------------------

	//描画ステートの作成
	m_states = std::make_unique<CommonStates>(device);

	//２D描画バッチの作成
	m_spriteBatch = std::make_unique<SpriteBatch>(context);

	//---------------------------------------------------
	//テクスチャ読み込み
	//---------------------------------------------------
	CreateWICTextureFromFile(device, L"Resources\\Textures\\Clear.png", nullptr, m_textureClear.GetAddressOf());
	CreateWICTextureFromFile(device, L"Resources\\Textures\\Over.png", nullptr, m_textureOver.GetAddressOf());

	CreateWICTextureFromFile(device, L"Resources\\Textures\\Retry.png", nullptr, m_textureRetry.GetAddressOf());
	CreateWICTextureFromFile(device, L"Resources\\Textures\\選択画面.png", nullptr, m_textureSelect.GetAddressOf());
	CreateWICTextureFromFile(device, L"Resources\\Textures\\タイトルに移動.png", nullptr, m_textureTitle.GetAddressOf());

	//ボタンUIテクスチャ読み込み
	CreateWICTextureFromFile(device,L"Resources\\Textures\\ButtonUI1.png",nullptr,m_textureButtonUI.GetAddressOf());

	//選択中の矢印テクスチャ読み込み
	CreateWICTextureFromFile(device,L"Resources\\Textures\\Cursor.png",nullptr,m_textureCursor.GetAddressOf());

	//---------------------------------------------------
	//モデル読み込み
	//---------------------------------------------------

	DirectX::EffectFactory fx(device);
	fx.SetDirectory(L"Resources\\Models");

	//プレイヤーのモデルを読みこみ
	m_resultModel = DirectX::Model::CreateFromSDKMESH(
		device, L"Resources\\Models\\Win2.sdkmesh",
		fx
	);

	//---------------------------------------------------
	//天候読み込み
	//---------------------------------------------------
	//雪の生成
	m_snow = std::make_unique<Snow>();
	m_snow->Initialize(device);
	//雨の生成
	m_rain = std::make_unique<Rain>();
	m_rain->Initialize(device);

	//---------------------------------------------------
	//BGM読み込み
	//---------------------------------------------------

	//BGM
	AudioManager* audio = AudioManager::GetInstance();
	audio->Initialize();

	//勝敗判定でBGM切り替え
	if (isClear)
	{
		audio->LoadSound("Result_Clear", L"Resources/Sounds/Light-Steps.wav");
		//音量
		audio->SetBGMVolume(0.2f);
		audio->PlayBGM("Result_Clear");
	}
	else
	{
		audio->LoadSound("Result_Over", L"Resources/Sounds/Tristesse.wav");
		//音量
		audio->SetBGMVolume(0.2f);
		audio->PlayBGM("Result_Over");
	}

	//SE 決定音
	audio->SetSEVolume(1.0f);
	audio->LoadSound("SE_Click", L"Resources/Sounds/決定ボタンを押す44.wav");

}

//-----------------------------------------------------------------
//ウィンドウサイズ関連
//-----------------------------------------------------------------
void ResultScene::CreateWindowSizeDependentResources()
{
	auto size = m_deviceResources->GetOutputSize();
	float aspectRatio = float(size.right - size.left) / float(size.bottom - size.top);

	//ビュー行列
	m_view = SimpleMath::Matrix::CreateLookAt(
		DirectX::SimpleMath::Vector3(0.0f, 2.0f, -6.0f),
		DirectX::SimpleMath::Vector3(0.0f, 1.0f, 0.0f),
		DirectX::SimpleMath::Vector3::Up
	);

	//射影行列
	m_proj = SimpleMath::Matrix::CreatePerspectiveFieldOfView(
		DirectX::XMConvertToRadians(45.f),
		aspectRatio,
		0.1f, 1000.f
	);
}

void ResultScene::OnDeviceLost()
{
	Finalize();
}
