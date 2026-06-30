/**
 * @file   SelectScene.cpp
 * @brief  選択画面の初期化・更新・描画を管理するクラス
 * @author 國田知睦
 * @date   2026/06/25
 */

//
#include "pch.h"
#include <Effects.h>
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


    //波を変更
    if (input->kbTracker.pressed.Tab)
    {
        m_waveManager->ToggleMode();
    }

	//ステージの数
    int stageCount = static_cast<int>(m_stageList.size());

	//カーソル移動（上キー）
    if (input->kbTracker.pressed.Up || input->kbTracker.pressed.W)
    {
		//移動音
		AudioManager::GetInstance()->Play("SE_Move");
		//カーソルを上に移動
        m_currentCursor--;
		//カーソルが0未満になったら、最後のステージにループ
        if (m_currentCursor < 0)
        {
			m_currentCursor = stageCount - 1; 
        }
    }

	//カーソル移動（下キー）
    if (input->kbTracker.pressed.Down || input->kbTracker.pressed.S)
    {
		//移動音
        AudioManager::GetInstance()->Play("SE_Move");
		//カーソルを下に移動
        m_currentCursor++;

		//カーソルがステージの数以上になったら、最初のステージにループ
        if (m_currentCursor >= stageCount)
        {
            m_currentCursor = 0; 
        }
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
        //現在カーソルの位置はステージが解放がされているか？
        if (IsStageUnlocked(m_currentCursor))
        {
            //決定音
            AudioManager::GetInstance()->Play("SE_Click");

            m_isChangingScene = true;
            transitionMask->Close();

            //選択したステージの種類をセット
            GamePlayScene::NextStageType = m_stageList[m_currentCursor].type;
        }
        else
        {
            //ステージが解放されていない音を追加
        }
    }

    //波の更新
    if (m_waveManager)
    {
        m_waveManager->Update(elapsedTime);
    }

    // ----------------------------------------------------
    // 回転アニメーションの計算
    // ----------------------------------------------------

    //アニメーション
    m_animationTimer += elapsedTime;

    float stepAngle = DirectX::XM_2PI / stageCount;

    //ステージの回転の計算
    m_targetAngle = GOAL_DIRECTION * m_currentCursor * stepAngle;

    //角度の差分(diff)を計算
    float diff = m_targetAngle - m_currentAngle;

    //最短ルートの計算
    while (diff > DirectX::XM_PI) diff -= DirectX::XM_2PI;
    while (diff < -DirectX::XM_PI) diff += DirectX::XM_2PI;

    //現在の角度を更新
    float speed = ROTATE_SPEED * elapsedTime;

    //合わせる
    m_currentAngle += diff * speed;

}

//-----------------------------------------------------------------
// 描画
//-----------------------------------------------------------------

void SelectScene::Render()
{
	//描画準備
	auto context = GetUserResources()->GetDeviceResources()->GetD3DDeviceContext();

    //波
    if (m_waveManager)
    {
        m_waveManager->Render(context, m_view, m_proj);
    }

    //円の半径
    float radius = TARGET_RADIUS;

    //サイズ設定
    float baseScale = MODEL_BASE_SCALE; //選んでいない
    float selectScale = MODEL_SELECCT_SCALE;//選択中

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
        // 行列の作成
        // ----------------------------------------------------

        //合わせる
        SimpleMath::Matrix world =
            SimpleMath::Matrix::CreateScale(scale) *
            SimpleMath::Matrix::CreateRotationY(selfRot) *
            SimpleMath::Matrix::CreateTranslation(x, 0.0f, z);

        // ----------------------------------------------------
        // 解放していないステージの色を変える
        // ----------------------------------------------------
        //描画の前にモデルの色を変更
        for (auto& mesh : m_stageList[i].model->meshes)
        {
            for (auto& part : mesh->meshParts)
            {
                //エフェクトを取得
                auto basicEffect = dynamic_cast<DirectX::BasicEffect*>(part->effect.get());

                if (basicEffect)
                {
                    //解放していない
                    if (!IsStageUnlocked(i))
                    {
                        //モデルの色を暗くする
                        basicEffect->SetDiffuseColor(Colors::Black);
                    }
                    else
                    {
                        //解放していたら元の色に戻すために白の明るさを付ける
                        basicEffect->SetDiffuseColor(Colors::White);
                    }
                }
            }
        }

        //描画
        m_stageList[i].model->Draw(
            context, *m_states, world, m_view, m_proj);
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

    if (m_textureWaveUI)
    {
        //波UI
		m_spriteBatch->Draw(m_textureWaveUI.Get(), SimpleMath::Vector2(0, 0));
    }
    
    // ----------------------------------------------------
    // テクスチャの描画
    // ----------------------------------------------------

    //テクスチャの座標
    float startX = UI_START_X;
    float startY = UI_START_Y;
    
    //カーソルのオフセット
    float cursorOffset = CURSOR_OFFSET;

    for (size_t i = 0; i < m_stageList.size(); i++)
    {


        //描画位置
        Vector2 position(startX, startY + (i * UI_STEP_Y));

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
            float scale = isSelected ? TEXTURE_SCALE_SELECTED : TEXTURE_SCALE_NORMAL;

            //選択中の色の変化
            XMVECTOR color;

            //ロック状態に合わせて色を変える
            if (!IsStageUnlocked(i))
            {
                //まだステージが解放されていない
                color = Colors::Black;
            }
            else
            {
                //ステージが解放されている場合は選択の色を変える
                color = isSelected ? Colors::White : Colors::Gray;
            }

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
                CURSOR_SCALE
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
                CURSOR_SCALE,
                SpriteEffects_FlipHorizontally
            );

        }

        //クリアマークの描画
        if (s_isClearedList[(int)m_stageList[i].type] && m_textureClearMark)
        {
            //クリアマーク画像のサイズの原点を計算
            Microsoft::WRL::ComPtr<ID3D11Resource> markRes;
            m_textureClearMark->GetResource(&markRes);
            CD3D11_TEXTURE2D_DESC markDesc;
            ((ID3D11Texture2D*)markRes.Get())->GetDesc(&markDesc);
            Vector2 markOrigin(markDesc.Width / 2.0f, markDesc.Height / 2.0f);

            Vector2 markPosition = position + Vector2(0.0f, 0.0f);

            m_spriteBatch->Draw(
                m_textureClearMark.Get(),
                markPosition,
                nullptr,
                Colors::White,
                0.0f,
                markOrigin,
                CLEAR_MARK_SCALE
            );
        }

    }

    m_spriteBatch->End();

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

	//ステージのデータをまとめる構造体
    struct  StageInDesc
    {
        std::wstring name;
		std::wstring texturePath;
        std::wstring modelPath;
		GamePlayScene::StageType type;
    };

	//ステージのデータの初期化用の配列
	//ステージ数を増やしたくなったらここに追加すればいい
    StageInDesc initDescs[] =
    {
        { L"Tutorial", L"Resources\\Textures\\Tutorial.png",  L"Resources\\Models\\Tutorial.sdkmesh", GamePlayScene::StageType::Tutorial },
        { L"EASY",     L"Resources\\Textures\\EASY_UI.png",   L"Resources\\Models\\S_1.sdkmesh",      GamePlayScene::StageType::EASY },
		{ L"NORMAL",   L"Resources\\Textures\\NORMAL_UI.png", L"Resources\\Models\\S_2.sdkmesh",      GamePlayScene::StageType::NORMAL },
		{ L"HARD",     L"Resources\\Textures\\HARD_UI.png",   L"Resources\\Models\\Hard.sdkmesh",      GamePlayScene::StageType::HARD },
    };

	//ステージのデータの初期化用の配列をループして、ステージのデータを構築する
    for (const auto& desc : initDescs)
    {
        StageData data;
        data.name = desc.name;
        DirectX::CreateWICTextureFromFile(device, desc.texturePath.c_str(), nullptr, data.texture.GetAddressOf());
        data.model = Model::CreateFromSDKMESH(device, desc.modelPath.c_str(), fx);
        data.type = desc.type;
        m_stageList.push_back(std::move(data));
    };

	//背景テクスチャ
    //ステージUI
    DirectX::CreateWICTextureFromFile(
        device,
        L"Resources\\Textures\\SelectUI.png",
        nullptr,
        m_backgroundTexture.GetAddressOf());
    //ボタンUI
    DirectX::CreateWICTextureFromFile(
        device,
        L"Resources\\Textures\\ButtonUI.png",
        nullptr,
        m_textureButtonUI.GetAddressOf());
    //カーソルUI
    DirectX::CreateWICTextureFromFile(
        device,
        L"Resources\\Textures\\Cursor.png",
        nullptr,
        m_textureCursor.GetAddressOf());
    //波変更UI
    DirectX::CreateWICTextureFromFile(
        device,
        L"Resources\\Textures\\WaveChange.png",
        nullptr,
        m_textureWaveUI.GetAddressOf());
    //クリアマークUI
    DirectX::CreateWICTextureFromFile(
        device,
        L"Resources\\Textures\\ClearMark.png",
        nullptr,
        m_textureClearMark.GetAddressOf());
    
    AudioManager* audio = AudioManager::GetInstance();
    audio->Initialize();
    audio->LoadSound("Select", L"Resources/Sounds/BGM_Title.wav");
    audio->SetBGMVolume(DEFAULT_BGM_VOLUME);
    audio->PlayBGM("Select");

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

void SelectScene::CreateWindowSizeDependentResources()
{

    HWND hwnd = m_deviceResources->GetWindow();
    RECT size = m_deviceResources->GetOutputSize();
    int width = size.right - size.left;
    int height = size.bottom - size.top;
    float aspectRatio = float(width) / float(height);

    //行列作成
    m_view = SimpleMath::Matrix::CreateLookAt(
        Vector3(0.0f, CAMERA_EYE_Y, CAMERA_EYE_Z),
        Vector3(0, CAMERA_TARGET_Y, 0),
        Vector3::Up
    );

    //射影行列を作成
    m_proj = SimpleMath::Matrix::CreatePerspectiveFieldOfView(
        XMConvertToRadians(CAMERA_FOV),
        aspectRatio,
        CAMERA_NEAR, CAMERA_FAR
    );

    //Waveの初期化
    m_waveManager = std::make_unique<WaveManager>(m_deviceResources);
    m_waveManager->Initialize(hwnd, width, height);

}

//-----------------------------------------------------------------
// デバイスロストした時に呼び出される関数
//-----------------------------------------------------------------

void SelectScene::OnDeviceLost()
{
	Finalize();
}

//-----------------------------------------------------------------
// 中心を基準にしてテクスチャを描画する関数
//-----------------------------------------------------------------

void SelectScene::DrawTextureCenter(
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

//-----------------------------------------------------------------
// ステージが解放されているかを判定するヘルパー関数
//-----------------------------------------------------------------

bool SelectScene::IsStageUnlocked(int stageIndex) const
{
    //最初のステージは常に遊べる
    if (stageIndex == 0)
    {
        return true;
    }

    //１つ前のステージタイプを取得
    auto prevStageType = m_stageList[stageIndex - 1].type;

    //1つ前のステージがクリアされていたら、このステージを解放
    return s_isClearedList[(int)prevStageType];
}
