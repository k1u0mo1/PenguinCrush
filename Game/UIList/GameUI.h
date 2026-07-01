
/**
 * @file   GameUI.h
 * @brief  画面のプレイヤーと敵のUIを表示を管理を行うクラス
 * @author 國田知睦
 * @date   2026/07/01
 */

#pragma once
#include "pch.h"
#include "Game/Common/DeviceResources.h"
#include "Game/PlayerList/Player.h"
#include "Game/EnemyList/BossEnemy.h"
#include <SpriteBatch.h>

/// <summary>
/// 画面のプレイヤーと敵のUIを表示するクラス
/// </summary>
class GameUI
{
private:

    //テクスチャのパス
    const wchar_t* TEX_PATH_WHITE= L"Resources\\Textures\\White.png";

    //プレイヤーUIの配置パラメータ
    const float   PLAYER_UI_X = 50.0f;
    const DirectX::SimpleMath::Vector2 PLAYER_HP_SIZE      = DirectX::SimpleMath::Vector2(300.0f, 30.0f);
    const DirectX::SimpleMath::Vector2 PLAYER_STAMINA_SIZE = DirectX::SimpleMath::Vector2(200.0f, 20.0f);

    //プレイヤーの最大体力
    const float PLAYER_MAX_HP = 300.0f;

    // 画面下からのオフセット
    const float   PLAYER_HP_BOTTOM_OFFSET = 90.0f; 
    const float   PLAYER_STAMINA_BOTTOM_OFFSET = 50.0f;

    //ボスUIの配置パラメータ
    const DirectX::SimpleMath::Vector2 BOSS_HP_SIZE = DirectX::SimpleMath::Vector2(400.0f, 30.0f);
    const float   BOSS_HP_Y = 10.0f;

    //ゲージを満タンで表示するための固定値
    const float   BAR_FULL = 1.0f;

public:

	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="deviceResources">デバイスリソースのポインタ</param>
	GameUI(DX::DeviceResources* deviceResources);

	/// <summary>
	/// デストラクタ
	/// </summary>
	~GameUI() = default;

	/// <summary>
	/// テクスチャの読み込みと初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// パラメータ系のUI描画
	/// </summary>
	/// <param name="player">プレイヤーのポインタ　パラメータを渡す</param>
	/// <param name="boss">敵のポインタ　パラメータを渡す</param>
	void Render(Player* player, BossEnemy* boss);

private:

    /// <summary>
    /// 指定された位置とサイズでゲージを描画する内部関数
    /// </summary>
    /// <param name="position">描画する左上の座標</param>
    /// <param name="current">現在の値</param>
    /// <param name="max">最大値</param>
    /// <param name="color">バーの色</param>
    /// <param name="scale">バーの最大サイズ</param>
    void DrawBar(
        const DirectX::SimpleMath::Vector2& position,
        float current,
        float max,
        const DirectX::SimpleMath::Color& color,
        const DirectX::SimpleMath::Vector2& scale
    );

private:

    DX::DeviceResources* m_deviceResources;

    // 2D描画用
    std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;
    std::unique_ptr<DirectX::CommonStates> m_states;

    // テクスチャ
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture;
};

