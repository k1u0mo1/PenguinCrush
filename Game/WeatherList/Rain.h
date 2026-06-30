
/**
 * @file   Rain.h
 * @brief  天候(雨)のクラス
 * @author 國田知睦
 * @date   2026/06/10
 */

#pragma once
#include "WeatherBase.h"

/// <summary>
/// 天候のクラス　雨
/// WeatherBaseの子クラス
/// </summary>
class Rain :public WeatherBase
{
private:

	//雨の落下速度
	static constexpr float RAIN_SPEED = 20.0f;
	//雨の太さ
	static constexpr float RAIN_WIDTH = 0.5f;
	//雨の描画数
	static constexpr UINT RAIN_PARTICLE_COUNT = 1000;

public:

	/// <summary>
	/// 初期化（オーバーライド）
	/// </summary>
	/// <param name="device">デバイス</param>
	void Initialize(ID3D11Device* device)override;

	/// <summary>
	/// 描画（オーバーライド）
	/// </summary>
	/// <param name="context">デバイスコンテキスト</param>
	/// <param name="view">ビュー行列</param>
	/// <param name="proj">射影行列</param>
	/// <param name="camPos">カメラの座標</param>
	void Render(ID3D11DeviceContext* context,
		const DirectX::SimpleMath::Matrix& view,
		const DirectX::SimpleMath::Matrix& proj,
		const DirectX::SimpleMath::Vector3& camPos) override;

private:

	// 雨専用のシェーダとレイアウト
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vs;
	Microsoft::WRL::ComPtr<ID3D11GeometryShader> m_gs;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_ps;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_layout;
};