
/**
 * @file   ResultScene.cpp
 * @brief  リザルト画面の初期化・更新・描画を管理するクラス
 * @author 國田知睦
 * @date   2026/07/01
 */

#include "pch.h"
#include "ResultScene.h"

//リザルトシーン->タイトルシーンに移動
#include "TitleScene.h"
#include "GamePlayScene.h"
#include "SelectScene.h"

//BGM
#include "Game/SoundList/AudioManager.h"

//-----------------------------------------------------------------
// コンストラクタ
//-----------------------------------------------------------------
ResultScene::ResultScene()
	: 
	m_deviceResources{},
	m_currentCursor(MenuType::Retry)
{
}

//-----------------------------------------------------------------
// 初期化
//-----------------------------------------------------------------
void ResultScene::Initialize()
{
	//デバイスリソース
	m_deviceResources = GetUserResources()->GetDeviceResources();
	//デバイスに依存するリソースを作成する関数
	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();

	//カーソルの位置をリセット
	m_currentCursor = MenuType::Retry;
	//シーンの移動
	m_isChangingScene = false;
	//アニメタイマー
	m_animationTimer = 0.0f;
	//フェードイン
	GetUserResources()->GetTransitionMask()->Open();
}

//-----------------------------------------------------------------
// 更新
//-----------------------------------------------------------------
void ResultScene::Update(float elapsedTime)
{
	elapsedTime;
	//キーボード用
	auto input = GetUserResources()->GetInputManager();

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
		//雪
		m_snow->Update(elapsedTime);
	}
	if (m_rain)
	{
		//雨
		m_rain->Update(elapsedTime);
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
		case MenuType::Retry:ChangeScene<GamePlayScene>(); break;
			//選択画面へ
		case MenuType::Select:ChangeScene<SelectScene>(); break;
			//タイトルへ
		case MenuType::Title:ChangeScene<TitleScene>(); break;
		}
	}
}

//-----------------------------------------------------------------
// 描画
//-----------------------------------------------------------------
void ResultScene::Render()
{
	//コンテキスト
	auto context = GetUserResources()->GetDeviceResources()->GetD3DDeviceContext();
	
	//-----------------------------------------
	// モデルの動き
	//-----------------------------------------
	DirectX::SimpleMath::Vector3 scale(MODEL_SCALE);   //サイズ
	DirectX::SimpleMath::Vector3 rotation(0.0f, 0.0f, 0.0f);//回転
	DirectX::SimpleMath::Vector3 position(0.0f, 0.0f, 0.0f);//座標

	//プレイヤーが勝った時
	if (GetUserResources()->IsGameClear())
	{
		//プレイヤーの動き
		position.y = abs(sinf(m_animationTimer * JUMP_SPEED)) * JUMP_HEIGHT;
		//回る
		rotation.y = m_animationTimer * CLEAR_MODEL_ROTATION_SPEED;
	}
	//負けたとき
	else
	{
		//倒れる
		rotation.x = -DirectX::XM_PIDIV2;//-90度
		rotation.y = DirectX::XM_PIDIV2; //90度
		//プレイヤーの座標
		position.y = OVER_MODEL_HEIGHT;
	}

	//-----------------------------------------
	// 行列で描画
	//-----------------------------------------
	//３Dモデル
	if (m_resultModel)
	{
		DirectX::SimpleMath::Matrix world =
			DirectX::SimpleMath::Matrix::CreateScale(scale) *
			DirectX::SimpleMath::Matrix::CreateRotationX(rotation.x) *
			DirectX::SimpleMath::Matrix::CreateRotationY(rotation.y) *
			DirectX::SimpleMath::Matrix::CreateRotationZ(rotation.z) *
			DirectX::SimpleMath::Matrix::CreateTranslation(position);
		//プレイヤーのモデルの描画
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
	//スプライトバッチ
	m_spriteBatch->Begin(DirectX::SpriteSortMode_Deferred, m_states->NonPremultiplied());

	//カメラの位置
	DirectX::SimpleMath::Vector3 eyePos(0.0f, WEATHER_CAMERA_POS_Y, WEATHER_CAMERA_POS_Z);
	//勝敗　勝ち
	if (GetUserResources()->IsGameClear())
	{
		DrawTextureCenter(
			m_textureClear.Get(),
			DirectX::SimpleMath::Vector2(
				centerX, screenH * RESULT_LOGO_POS_Y_RATIO), 2.0f);
		//雪
		if (m_snow)
		{
			m_snow->Render(context, m_view, m_proj, eyePos);
		}
	}
	//負け
	else
	{
		DrawTextureCenter(
			m_textureOver.Get(),
			DirectX::SimpleMath::Vector2(
				centerX, screenH * RESULT_LOGO_POS_Y_RATIO), 2.0f);
		//雨
		if (m_rain)
		{
			m_rain->Render(context, m_view, m_proj, eyePos);
		}
	}

	// ----------------------------------------------------
	//  ボタンUIテクスチャの描画
	// ----------------------------------------------------
	if (m_textureButtonUI)
	{
		//ボタンUI
		m_spriteBatch->Draw(m_textureButtonUI.Get(), DirectX::SimpleMath::Vector2(0, 0));
	}
	// ボタンを表示する基準の高さ
	float startY = screenH * BUTTON_START_Y_RATIO;
	float stepY = BUTTON_STEP_Y; // ボタンごとの間隔

	//テクスチャを配列にまとめる
	ID3D11ShaderResourceView* buttonTextures[] =
	{
		m_textureRetry.Get(),
		m_textureSelect.Get(),
		m_textureTitle.Get()
	};
	//テクスチャを配列でまとめたものを一か所で描画を行う
	for (int i = 0;i < static_cast<int>(MenuType::Count); i++)
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
		DrawTextureCenter(
			m_textureCursor.Get(),
			DirectX::SimpleMath::Vector2(centerX - CURSOR_OFFSET_X, cursorY),
			CURSOR_SCALE);
		//右側の矢印 反転
		DrawTextureCenter(
			m_textureCursor.Get(),
			DirectX::SimpleMath::Vector2(centerX + CURSOR_OFFSET_X, cursorY),
			CURSOR_SCALE, DirectX::Colors::White, 
			DirectX::SpriteEffects_FlipHorizontally);
	}

	m_spriteBatch->End();
}

//-----------------------------------------------------------------
// 終了処理
//-----------------------------------------------------------------
void ResultScene::Finalize()
{
}

//-----------------------------------------------------------------
// デバイスに依存するリソースを作成する関数
//-----------------------------------------------------------------
void ResultScene::CreateDeviceDependentResources()
{
	auto device = GetUserResources()->GetDeviceResources()->GetD3DDevice();
	auto context = GetUserResources()->GetDeviceResources()->GetD3DDeviceContext();

	//---------------------------------------------------
	//共通ステートとバッチ
	//---------------------------------------------------
	//描画ステートの作成
	m_states = std::make_unique<DirectX::CommonStates>(device);
	//２D描画バッチの作成
	m_spriteBatch = std::make_unique<DirectX::SpriteBatch>(context);

	//---------------------------------------------------
	//テクスチャ読み込み
	//---------------------------------------------------
	DirectX::CreateWICTextureFromFile(device, L"Resources\\Textures\\Clear.png", nullptr, m_textureClear.GetAddressOf());
	DirectX::CreateWICTextureFromFile(device, L"Resources\\Textures\\Over.png",  nullptr, m_textureOver.GetAddressOf());
	DirectX::CreateWICTextureFromFile(device, L"Resources\\Textures\\Retry.png", nullptr, m_textureRetry.GetAddressOf());
	DirectX::CreateWICTextureFromFile(device, L"Resources\\Textures\\選択画面.png",       nullptr, m_textureSelect.GetAddressOf());
	DirectX::CreateWICTextureFromFile(device, L"Resources\\Textures\\タイトルに移動.png", nullptr, m_textureTitle.GetAddressOf());

	//ボタンUIテクスチャ読み込み
	DirectX::CreateWICTextureFromFile(device,L"Resources\\Textures\\ButtonUI1.png",nullptr,m_textureButtonUI.GetAddressOf());
	//選択中の矢印テクスチャ読み込み
	DirectX::CreateWICTextureFromFile(device,L"Resources\\Textures\\Cursor.png",nullptr,m_textureCursor.GetAddressOf());

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
	//勝ち
	if (GetUserResources()->IsGameClear())
	{
		audio->LoadSound("Result_Clear", L"Resources/Sounds/BGM_Clear.wav");
		//音量
		audio->SetBGMVolume(DEFAULT_BGM_VOLUME);
		audio->PlayBGM("Result_Clear");
	}
	//負け
	else
	{
		audio->LoadSound("Result_Over", L"Resources/Sounds/BGM_Over.wav");
		//音量
		audio->SetBGMVolume(DEFAULT_BGM_VOLUME);
		audio->PlayBGM("Result_Over");
	}

	//SE 決定音
	audio->SetSEVolume(DEFAULT_SE_CLICK_VOLUME);
	audio->LoadSound("SE_Click", L"Resources/Sounds/SE_Click.wav");
	//SE 移動音
	audio->SetSEVolume(DEFAULT_SE_VOLUME);
	audio->LoadSound("SE_Move", L"Resources/Sounds/SE_MoveCursor.wav");
}

//-----------------------------------------------------------------
// ウインドウサイズに依存するリソースを作成する関数
//-----------------------------------------------------------------
void ResultScene::CreateWindowSizeDependentResources()
{
	auto size = m_deviceResources->GetOutputSize();
	float aspectRatio = float(size.right - size.left) / float(size.bottom - size.top);

	//ビュー行列
	m_view = DirectX::SimpleMath::Matrix::CreateLookAt(
		DirectX::SimpleMath::Vector3(0.0f, CAMERA_EYE_Y, CAMERA_EYE_Z),
		DirectX::SimpleMath::Vector3(0.0f, CAMERA_TARGET_Y, 0.0f),
		DirectX::SimpleMath::Vector3::Up
	);
	//射影行列
	m_proj = DirectX::SimpleMath::Matrix::CreatePerspectiveFieldOfView(
		DirectX::XMConvertToRadians(CAMERA_FOV),
		aspectRatio,
		CAMERA_NEAR, CAMERA_FAR
	);
}

//-----------------------------------------------------------------
// デバイスロストした時に呼び出される関数
//-----------------------------------------------------------------

void ResultScene::OnDeviceLost()
{
	Finalize();
}

//-----------------------------------------------------------------
// 中心を基準にしてテクスチャを描画する関数
//-----------------------------------------------------------------

void ResultScene::DrawTextureCenter(
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
