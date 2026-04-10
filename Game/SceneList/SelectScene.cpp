

//タイトルシーンクラス


//
#include "pch.h"
#include "SelectScene.h"

//選択シーンー＞ゲームプレイシーンに移動
#include "GamePlayScene.h"
#include "TitleScene.h"

using namespace DirectX;

//-----------------------------------------------------------------
// コンストラクタ
//-----------------------------------------------------------------

SelectScene::SelectScene()
    : m_deviceResources()
{

}

//-----------------------------------------------------------------
// 初期化
//-----------------------------------------------------------------

void SelectScene::Initialize()
{

    m_deviceResources = GetUserResources()->GetDeviceResources();

	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();

    //カーソルリセット
    m_currentCursor = 0;

    // フェードイン
    GetUserResources()->GetTransitionMask()->Open();

}

//-----------------------------------------------------------------
// 更新
//-----------------------------------------------------------------

void SelectScene::Update(float elapsedTime)
{
	elapsedTime;

	auto input = GetUserResources()->GetInputManager();

    // ----------------------------------------------------
    // カーソル移動（上下キー）
    // ----------------------------------------------------
    if (input->kbTracker.pressed.Up || input->kbTracker.pressed.W)
    {
        //移動音
        AudioManager::GetInstance()->Play("SE_Move");

        m_currentCursor--;

        if (m_currentCursor < 0)
            m_currentCursor = MENU_COUNT - 1; // 下からループ
    }

    if (input->kbTracker.pressed.Down || input->kbTracker.pressed.S)
    {
        //移動音
        AudioManager::GetInstance()->Play("SE_Move");

        m_currentCursor++;

        if (m_currentCursor >= MENU_COUNT)
            m_currentCursor = 0; // 上からループ
    }

    auto transitionMask = GetUserResources()->GetTransitionMask();

    //フェード
    if (m_isChangingScene)
    {
        if (transitionMask->IsClose() && transitionMask->IsEnd())
        {
            // ゲームシーンへ移動
            ChangeScene<GamePlayScene>();
        }
        return;
    }

    //[T]で戻る
    if (input->kbTracker.pressed.T)
    {
        ChangeScene<TitleScene>();
    }

    //[o]でタイトルへ強制戻る
    if (input->kbTracker.pressed.O)
    {
        ChangeScene<TitleScene>();
    }

    // ----------------------------------------------------
    // 決定（エンターキー）
    // ----------------------------------------------------
    if (input->kbTracker.pressed.Enter || input->kbTracker.pressed.Space)
    {

        //決定音
        AudioManager::GetInstance()->Play("SE_Click");

        m_isChangingScene = true;
        transitionMask->Close();

        // 選ばれているカーソルによって、次のステージの種類をセットする
        switch (m_currentCursor)
        {
        case 0: // チュートリアル
            GamePlayScene::NextStageType = GamePlayScene::StageType::Tutorial;
            break;

        case 1: // ステージ1（本番）
            GamePlayScene::NextStageType = GamePlayScene::StageType::Stage1;
            break;

        case 2: // ステージ1（本番）
            GamePlayScene::NextStageType = GamePlayScene::StageType::Stage2;
            break;
        }

        // ゲームシーンへ移動
        //ChangeScene<GamePlayScene>();
    }

    //波の更新
    if (m_wave)
    {
        m_wave->Update(elapsedTime);
    }

    // ----------------------------------------------------
    // 回転アニメーションの計算
    // ----------------------------------------------------

    //アニメーション
    m_animationTimer += elapsedTime;

    float stepAngle = DirectX::XM_2PI / MENU_COUNT;

    //目標の角度の計算
    float direction = 1.0f;

    m_targetAngle = direction * m_currentCursor * stepAngle;

    //角度の差分(diff)を計算
    float diff = m_targetAngle - m_currentAngle;

    //最短ルートの計算
    while (diff > DirectX::XM_PI) diff -= DirectX::XM_2PI;
    while (diff < -DirectX::XM_PI) diff += DirectX::XM_2PI;

    //現在の角度を更新
    float speed = 5.0f * elapsedTime;

    //合わせる
    m_currentAngle += diff * speed;

}

//-----------------------------------------------------------------
// 描画
//-----------------------------------------------------------------

void SelectScene::Render()
{
	/*auto debugFont = GetUserResources()->GetDebugFont();*/

	////どのシーンか描画する
	//debugFont->AddString(L"SelectScene", SimpleMath::Vector2(0.0f, debugFont->GetFontHeight()));
	/*debugFont->AddString(L"ChangeScene: 1 true", SimpleMath::Vector2(0.0f, 60.0f));
	debugFont->AddString(L"ChangeScene: 2 false", SimpleMath::Vector2(0.0f, 80.0f));
	debugFont->AddString(L"ChangeScene: 3 false", SimpleMath::Vector2(0.0f, 100.0f));*/

	auto context = GetUserResources()->GetDeviceResources()->GetD3DDeviceContext();
	/*auto states = GetUserResources()->GetCommonStates();*/

    //波
    if (m_wave)
    {
        m_wave->Render(context, m_view, m_proj);
    }

    //円の半径
    float radius = 4.0f;

    //サイズ設定
    float baseScale = 0.3f; //選んでいない
    float selectScale = 0.5f;//選択中

    //モデル（ステージ）の１つ当たりの角度の間隔
    float stepAngle = DirectX::XM_2PI / m_stageList.size();

    // ----------------------------------------------------
    // モデル描画のループ処理
    // ----------------------------------------------------

    for (int i = 0; i < m_stageList.size(); i++)
    {
        //モデルがロードされていなかったらスキップする
        if (!m_stageList[i].model)continue;

        // ----------------------------------------------------
        // 角度と座標の計算
        // ----------------------------------------------------

        //回転角
        float angle = m_currentAngle - (i * stepAngle);

        float x =  sinf(angle) * radius;
        float z = -cosf(angle) * radius;

        // ----------------------------------------------------
        // スケールと回転の計算
        // ----------------------------------------------------

        //自分が選択されているか？
        bool isSelected = (i == m_currentCursor);

        //選択されていたら大きくなる　それ以外は小さく　表示
        float scale = isSelected ? selectScale : baseScale;

        //モデル自身の回転（演出用）
        float selfRot = m_animationTimer;

        // ----------------------------------------------------
        // 行列の作成と描画
        // ----------------------------------------------------

        //合わせる
        SimpleMath::Matrix world =
            SimpleMath::Matrix::CreateScale(scale) *
            SimpleMath::Matrix::CreateRotationY(selfRot) *
            SimpleMath::Matrix::CreateTranslation(x, 0.0f, z);

        //描画
        m_stageList[i].model->Draw(context, *m_states, world, m_view, m_proj);

    }

    m_spriteBatch->Begin(SpriteSortMode_Deferred, m_states->NonPremultiplied());

    // ----------------------------------------------------
    // 背景テクスチャの描画
    // ----------------------------------------------------

    if (m_backgroundTexture)
    {
        //画面全体に描画 UI
        m_spriteBatch->Draw(m_backgroundTexture.Get(), SimpleMath::Vector2(0, 0));
        
    }

    if (m_textureButtonUI)
    {
        //ボタンUI
        m_spriteBatch->Draw(m_textureButtonUI.Get(), SimpleMath::Vector2(0, 0));
    }

    
    // ----------------------------------------------------
    // テクスチャの描画
    // ----------------------------------------------------

    //テクスチャの座標
    float startX = 150.0f;
    float startY = 150.0f;
    //テクスチャ同士の間隔
    float gapY = 60.0f;

    //カーソルのオフセット
    float cursorOffset = 120.0f;

    for (size_t i = 0; i < m_stageList.size(); i++)
    {
        //描画位置
        Vector2 position(startX, startY + (i * gapY));

        //何番か
        bool isSelected = (i == m_currentCursor);

        //テクスチャの描画
        if (m_stageList[i].texture)
        {
            Microsoft::WRL::ComPtr<ID3D11Resource> res;
            m_stageList[i].texture->GetResource(&res);
            CD3D11_TEXTURE2D_DESC desc;

            ((ID3D11Texture2D*)res.Get())->GetDesc(&desc);

            Vector2 origin(desc.Width / 2.0f, desc.Height / 2.0f);

            //選択中は大きくさせる
            float scale = isSelected ? 0.8f : 0.5f;

            //選択中の色の変化
            XMVECTOR color = isSelected ? Colors::White : Colors::Gray;

            m_spriteBatch->Draw(
                m_stageList[i].texture.Get(), //画像
                position,                     //位置
                nullptr,                      //画像の切り取り範囲
                color,                        //色
                0.0f,                         //回転角度
                origin,                       //原点
                scale                         //大きさ
            );
        }

        //カーソル
        if (isSelected && m_textureCursor)
        {
            //カーソル画像の情報を取得
            Microsoft::WRL::ComPtr<ID3D11Resource> curRes;
            m_textureCursor->GetResource(&curRes);

            CD3D11_TEXTURE2D_DESC curDesc;
            ((ID3D11Texture2D*)curRes.Get())->GetDesc(&curDesc);
            SimpleMath::Vector2 curOrigin(curDesc.Width / 2.0f, curDesc.Height / 2.0f);


            //左カーソル
            SimpleMath::Vector2 leftPos = position;
            leftPos.x -= cursorOffset;

            m_spriteBatch->Draw(
                m_textureCursor.Get(),
                leftPos,
                nullptr,
                Colors::White,
                0.0f,
                curOrigin,
                0.25f
            );


            //右カーソル
            SimpleMath::Vector2 rightPos = position;
            rightPos.x += cursorOffset;

            m_spriteBatch->Draw(
                m_textureCursor.Get(),
                rightPos,
                nullptr,
                Colors::White,
                0.0f,
                curOrigin,
                0.25f,
                SpriteEffects_FlipHorizontally
            );

        }


    }



    m_spriteBatch->End();

    //// ----------------------------------------------------
    //// メニュー項目の描画
    //// ----------------------------------------------------
    //float startY = 150.0f;
    //float lineHeight = 40.0f;

    //// 表示する文字のリスト
    //const wchar_t* menuItems[] = {
    //    L"Tutorial Mode", // 0番
    //    L"Game Start" ,   // 1番
    //    L"Game2 Start"     // 2番
    //};

    //for (int i = 0; i < MENU_COUNT; ++i)
    //{
    //    std::wstring text = menuItems[i];

    //    // 今選んでいるやつだけ矢印をつける
    //    if (i == m_currentCursor)
    //    {
    //        text = L"> " + text + L" <"; // 強調表示
    //    }
    //    else
    //    {
    //        text = L"  " + text;       // 普通の表示
    //    }

    //    debugFont->AddString(text.c_str(), Vector2(100.0f, startY + i * lineHeight));
    //}

}

//-----------------------------------------------------------------
// 終了処理
//-----------------------------------------------------------------

void SelectScene::Finalize()
{

}

//-----------------------------------------------------------------
// デバイスに依存するリソースを作成する関数
//-----------------------------------------------------------------

void SelectScene::CreateDeviceDependentResources()
{
	auto device = GetUserResources()->GetDeviceResources()->GetD3DDevice();
	auto context = GetUserResources()->GetDeviceResources()->GetD3DDeviceContext();
	
    //-------------------------------------------------
    //基本リソース
    //-------------------------------------------------
   
    //スプライトバッチ
    m_spriteBatch = std::make_unique<SpriteBatch>(context);
    //ステート
    m_states = std::make_unique<CommonStates>(device);

    //-------------------------------------------------
    //ステージのデータの構築
    //-------------------------------------------------

    DirectX::EffectFactory fx(device);
    fx.SetDirectory(L"Resources\\Models");

    //再初期化の時用にクリア
    m_stageList.clear();

    //Tutorial
    {
        StageData data;
        data.name = L"Tutorial";
        CreateWICTextureFromFile(device, L"Resources\\Textures\\Tutorial.png", nullptr, data.texture.GetAddressOf());
        data.model = Model::CreateFromSDKMESH(device, L"Resources\\Models\\Tutorial.sdkmesh", fx);
        data.type = GamePlayScene::StageType::Tutorial;
        m_stageList.push_back(std::move(data));
    }
    //Stage1
    {
        StageData data;
        data.name = L"Stage1";
        CreateWICTextureFromFile(device, L"Resources\\Textures\\Stage.png", nullptr, data.texture.GetAddressOf());
        data.model = Model::CreateFromSDKMESH(device, L"Resources\\Models\\Stage1.sdkmesh", fx);
        data.type = GamePlayScene::StageType::Stage1;
        m_stageList.push_back(std::move(data));
    }
    //Stage2
    {
        StageData data;
        data.name = L"Stage2";
        CreateWICTextureFromFile(device, L"Resources\\Textures\\Stage2.png", nullptr, data.texture.GetAddressOf());
        data.model = Model::CreateFromSDKMESH(device, L"Resources\\Models\\Stage1.sdkmesh", fx);
        data.type = GamePlayScene::StageType::Stage2;
        m_stageList.push_back(std::move(data));
    }

    //テクスチャ読み込み
    CreateWICTextureFromFile(
        device,
        L"Resources\\Textures\\SelectUI.png",
        nullptr,
        m_backgroundTexture.GetAddressOf());

    //ボタンUIテクスチャ読み込み
    CreateWICTextureFromFile(
        device,
        L"Resources\\Textures\\ButtonUI.png",
        nullptr,
        m_textureButtonUI.GetAddressOf());

    //選択中の矢印テクスチャ読み込み
    CreateWICTextureFromFile(
        device,
        L"Resources\\Textures\\Cursor.png",
        nullptr,
        m_textureCursor.GetAddressOf());

    
    AudioManager* audio = AudioManager::GetInstance();
    audio->Initialize();
    audio->LoadSound("Select", L"Resources/Sounds/BGM_Title.wav");
    audio->SetBGMVolume(0.2f);
    audio->PlayBGM("Select");

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

void SelectScene::CreateWindowSizeDependentResources()
{

    HWND hwnd = m_deviceResources->GetWindow();
    RECT size = m_deviceResources->GetOutputSize();
    int width = size.right - size.left;
    int height = size.bottom - size.top;
    float aspectRatio = float(width) / float(height);

    //行列作成
    m_view = SimpleMath::Matrix::CreateLookAt(
        Vector3(0.0f, 5.0f, -11.0f),
        Vector3(0, 0.0f, 0),
        Vector3::Up
    );

    //射影行列を作成
    m_proj = SimpleMath::Matrix::CreatePerspectiveFieldOfView(
        XMConvertToRadians(45.0f),
        aspectRatio,
        0.1f, 1000.0f
    );

    //Waveの初期化
    m_wave = std::make_unique<Wave>(m_deviceResources);
    m_wave->Initialize(hwnd, width, height);

}

//-----------------------------------------------------------------
// デバイスロストした時に呼び出される関数
//-----------------------------------------------------------------

void SelectScene::OnDeviceLost()
{
	Finalize();
}
