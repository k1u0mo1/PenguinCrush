
/**
 * @file   GameUI.cpp
 * @brief  画面のプレイヤーと敵のUIを表示を管理を行うクラス
 * @author 國田知睦
 * @date   2026/07/01
 */

#include "pch.h"
#include "GameUI.h"
//テクスチャ読み込み用
#include <WICTextureLoader.h>

//----------------------------------------------------------
// コンストラクタ
//----------------------------------------------------------

GameUI::GameUI(DX::DeviceResources* deviceResources)
    : 
    m_deviceResources(deviceResources)
{
}

//----------------------------------------------------------
// テクスチャの読み込みと初期化
//----------------------------------------------------------

void GameUI::Initialize()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto context = m_deviceResources->GetD3DDeviceContext();
    //スプライトバッチと共通ステートの作成
    m_spriteBatch = std::make_unique<DirectX::SpriteBatch>(context);
    m_states = std::make_unique<DirectX::CommonStates>(device);

    DirectX::CreateWICTextureFromFile(
        device,
        TEX_PATH_WHITE,
        nullptr,
        m_texture.GetAddressOf()
    );
}

//----------------------------------------------------------
// パラメータ系のUI描画
//----------------------------------------------------------

void GameUI::Render(Player* player, BossEnemy* boss)
{
    // 画面サイズを取得
    RECT rectSize = m_deviceResources->GetOutputSize();
    /*float screenWidth = static_cast<float>(rectSize.right - rectSize.left);*/
    float screenHeight = static_cast<float>(rectSize.bottom - rectSize.top);

    //スプライト描画開始
    m_spriteBatch->Begin(DirectX::SpriteSortMode_Deferred, m_states->NonPremultiplied());

    // ---------------------------------------------------
    // プレイヤーのバー (画面左下)
    // ---------------------------------------------------

    if (player)
    {
        // 座標の計算 
        // HP
        DirectX::SimpleMath::Vector2 hpPos = { PLAYER_UI_X, screenHeight - PLAYER_HP_BOTTOM_OFFSET };
        // スタミナ
        DirectX::SimpleMath::Vector2 stPos = { PLAYER_UI_X, screenHeight - PLAYER_STAMINA_BOTTOM_OFFSET };
        // ---------------------------------------------------
        // プレイヤーのHPバー (画面左下)
        // ---------------------------------------------------
        //背景（黒）
        DrawBar(
            hpPos,
            BAR_FULL,
            BAR_FULL,
            DirectX::SimpleMath::Color(DirectX::Colors::Black),
            PLAYER_HP_SIZE
        );

        // HP（緑）
        DrawBar(
            hpPos,
            player->GetHP(),
            PLAYER_MAX_HP,
            DirectX::SimpleMath::Color(DirectX::Colors::Lime),
            PLAYER_HP_SIZE
        ); 

        // ---------------------------------------------------
        // プレイヤーのスタミナバー (画面左下)
        // ---------------------------------------------------
        // スタミナ（黄色） - HPの下に表示する
        DrawBar(
            stPos,
            BAR_FULL,
            BAR_FULL,
            DirectX::SimpleMath::Color(DirectX::Colors::Black),
            PLAYER_STAMINA_SIZE
        );
        DrawBar(
            stPos,
            player->GetStamina(),
            player->GetMaxStamina(),
            DirectX::SimpleMath::Color(DirectX::Colors::Yellow),
            PLAYER_STAMINA_SIZE
        );
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
        DirectX::SimpleMath::Vector2 pos = { centerX - (BOSS_HP_SIZE.x/2.0f), BOSS_HP_Y}; // 中央から左にずらす

        // 背景
        DrawBar(pos, 
            BAR_FULL,
            BAR_FULL,
            DirectX::SimpleMath::Color(DirectX::Colors::Black),
            BOSS_HP_SIZE
        );
        // HP（赤）
        DrawBar(pos,
            boss->GetHP(),
            bossMaxHP,
            DirectX::SimpleMath::Color(DirectX::Colors::Red),
            BOSS_HP_SIZE
        );
    }
    // 描画終了
    m_spriteBatch->End();
}

//----------------------------------------------------------
// 指定された位置とサイズでゲージを描画する内部関数
//----------------------------------------------------------

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
