
//プレイヤーと敵の UI

#pragma once

#include "pch.h"
#include "Common/DeviceResources.h"

#include "PlayerList/Player.h"
#include "EnemyList/BossEnemy.h"

#include <SpriteBatch.h>

/// <summary>
/// 画面のプレイヤーと敵のUIを表示するクラス
/// </summary>
class GameUI
{
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

