
//雨　Rain.h

#pragma once
#include "WeatherBase.h"

/// <summary>
/// 天候のクラス　雨
/// WeatherBaseの子クラス
/// </summary>
class Rain :public WeatherBase
{
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