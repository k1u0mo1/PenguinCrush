#include "pch.h"
#include "GameUI.h"

//テクスチャ読み込み用
#include <WICTextureLoader.h>

using namespace DirectX;
using namespace DirectX::SimpleMath;

GameUI::GameUI(DX::DeviceResources* deviceResources)
    : m_deviceResources(deviceResources)
{
}

void GameUI::Initialize()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto context = m_deviceResources->GetD3DDeviceContext();

    m_spriteBatch = std::make_unique<SpriteBatch>(context);
    m_states = std::make_unique<CommonStates>(device);

    
    CreateWICTextureFromFile(
        device,
        L"Resources\\Textures\\White.png",
        nullptr,
        m_texture.GetAddressOf()
    );
}

void GameUI::Render(Player* player, BossEnemy* boss)
{
    /*auto context = m_deviceResources->GetD3DDeviceContext();*/

    // 画面サイズを取得
    RECT rectSize = m_deviceResources->GetOutputSize();
    /*float screenWidth = static_cast<float>(rectSize.right - rectSize.left);*/
    float screenHeight = static_cast<float>(rectSize.bottom - rectSize.top);

    //スプライト描画開始
    m_spriteBatch->Begin(SpriteSortMode_Deferred, m_states->NonPremultiplied());

    // ---------------------------------------------------
    // プレイヤーのバー (画面左下)
    // ---------------------------------------------------

    if (player)
    {
        // 座標の計算 
        // HP
        Vector2 hpPos = { 50.0f, screenHeight - 90.0f };
        // スタミナ
        Vector2 stPos = { 50.0f, screenHeight - 50.0f };
        // 弾数
        Vector2 amPos = { 50.0f, screenHeight - 20.0f };

        // ---------------------------------------------------
        // プレイヤーのHPバー (画面左下)
        // ---------------------------------------------------
        //背景（黒）
        DrawBar(
            hpPos,
            100, 
            100,
            Color(Colors::Black),
            { 300,30 }
        );

        // HP（緑）
        DrawBar(
            hpPos,
            player->GetHP(),
            300.0f, // 100.0fは最大HP
            Color(Colors::Lime),
            { 300, 30 }
        ); 

        // ---------------------------------------------------
        // プレイヤーのスタミナバー (画面左下)
        // ---------------------------------------------------

        // スタミナ（黄色） - HPの下に表示する
        DrawBar(
            stPos,
            100,
            100,
            Color(Colors::Black),
            { 200, 20 }
        );

        DrawBar(
            stPos,
            player->GetStamina(),
            100.0f,
            Color(Colors::Yellow),
            { 200, 20 }
        );

        // ---------------------------------------------------
        // プレイヤーの弾数バー (画面左下)
        // ---------------------------------------------------
        //現在の弾数
        int currentAmmo = player->GetAmmo();
        //最大弾数
        int maxAmmo = player->GetMaxAmmo();

        //１発当たりのサイズと間隔
        float bulletWidth = 10.0f; //幅
        float bulletHeight = 15.0f;//高さ
        float gap = 4.0f;          //間隔 

        for (int i = 0; i < maxAmmo; ++i)
        {
            // 描画位置を計算（横にずらしていく）
            Vector2 drawPos = amPos;
            drawPos.x += i * (bulletWidth + gap);

            DirectX::SimpleMath::Color color;
            if (i < currentAmmo)
            {
                // 残っている弾の色
                color = DirectX::Colors::Blue; 
            }
            else
            {
                // 空きスロットの色
                color = DirectX::Colors::Gray * 0.3f;

            }

            DrawBar(
                drawPos,
                1.0f, 1.0f,    // 常に満タン扱いにしてサイズを固定する
                color,
                { bulletWidth, bulletHeight }
            );
        }
    }

    //---------------------------------------------------
    // ボスのHPバー (画面上部中央)
    //---------------------------------------------------
    if (boss && !boss->IsDead())
    {
        //ボスの最大HPを渡す
        float bossMaxHP = boss->GetMaxHP(); 

        // 画面中央計算
        RECT size = m_deviceResources->GetOutputSize();
        float centerX = (size.right - size.left) / 2.0f;
        Vector2 pos = { centerX - 200.0f, 10.0f }; // 中央から左にずらす

        // 背景
        DrawBar(pos, 
            1.0f,
            1.0f,
            Color(Colors::Black), 
            { 400, 30 }
        );
        // HP（赤）
        DrawBar(pos,
            boss->GetHP(),
            bossMaxHP,
            Color(Colors::Red), 
            { 400, 30 }
        );
    }


    //---------------------------------------------------
    // 酔い止めカーソル (画面中央)
    //---------------------------------------------------
    //画面の中心座標
    //float centerX = screenWidth / 2.0f - 5.0f;
    //float centerY = screenHeight / 2.0f - 30.0f;

    if (m_texture)
    {
        // 点のサイズ
        //float dotSize = 0.5f;

      
        //// White.png 
        //m_spriteBatch->Draw(
        //    m_texture.Get(),                 // テクスチャ
        //    Vector2(centerX, centerY),       // 位置（画面中央）
        //    nullptr,                         // ソース矩形（全体）
        //    Colors::White * 1.0f,            // 色（少し半透明にすると邪魔にならない：0.8f）
        //    0.0f,                            // 回転
        //    Vector2(0.5f, 0.5f),             // 原点（画像の中心）※1x1の画像ならこれでOK
        //    dotSize,                         // スケール（サイズ）
        //    SpriteEffects_None,
        //    0.0f
        //);
    }


    // 描画終了
    m_spriteBatch->End();
}

void GameUI::DrawBar(
    const DirectX::SimpleMath::Vector2& position,
    float current,
    float max, 
    const DirectX::SimpleMath::Color& color,
    const DirectX::SimpleMath::Vector2& scale)
{
    if (!m_texture) return;

    //比較を計算（0.0～1.0）
    float ratio = current / max;
    if (ratio < 0.0f)ratio = 0.0f;
    if (ratio > 1.0f)ratio = 1.0f;

    // 描画する矩形
    RECT drawRect = {};
    drawRect.left = 0;
    drawRect.top = 0;
    drawRect.right = static_cast<LONG>(scale.x * ratio); // 横幅×比率
    drawRect.bottom = static_cast<LONG>(scale.y);        // 高さはそのまま

    // 単色の白画像を、指定したサイズと色で描画する
    // destinationRect (画面上の表示位置とサイズ)
    RECT destRect = {
        (LONG)position.x,
        (LONG)position.y,
        (LONG)(position.x + scale.x * ratio),
        (LONG)(position.y + scale.y)
    };

    m_spriteBatch->Draw(
        m_texture.Get(),
        destRect,
        nullptr, // ソース矩形（全体を使う）
        color
    );
}
