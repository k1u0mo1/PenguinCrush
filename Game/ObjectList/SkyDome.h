
/**
 * @file   SkyDome.h
 * @brief  スカイドームの描画・管理を行うクラス
 * @author 國田知睦
 * @date   2026/06/08
 */

#pragma once

#include "pch.h"

#include "Game/Common/DeviceResources.h"
#include "Game/Common/StepTimer.h"
#include <Model.h>


/// <summary>
/// 背景のスカイドームを描画・管理するクラス
/// </summary>
class SkyDome
{
private:

	//空の球体の大きさ
	static constexpr float DEFAULT_SCALE = 250.0f;

	//球体の高さを合わせるためのY軸オフセット
	static constexpr float HEIGHT_OFFSET_Y = -20.0f;

public:

	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="deviceResources">デバイスリソース</param>
	SkyDome(DX::DeviceResources* deviceResources);

	/// <summary>
	/// モデルの初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// モデルの描画
	/// </summary>
	/// <param name="context">デバイスコンテキスト</param>
	/// <param name="view">ビュー行列</param>
	/// <param name="proj">射影行列</param>
	void Render(
		ID3D11DeviceContext* context,
		const DirectX::SimpleMath::Matrix& view,
		const DirectX::SimpleMath::Matrix& proj
	);

private:

	//リソース管理
	DX::DeviceResources* m_deviceResources;

	//スカイドームのモデル
	std::unique_ptr<DirectX::Model> m_model;

	//描画ステート
	std::unique_ptr<DirectX::CommonStates> m_states;

	//エフェクトファクトリー
	std::unique_ptr<DirectX::EffectFactory> m_effectFactory;

	//カメラの位置
	DirectX::SimpleMath::Vector3 m_position;

};
