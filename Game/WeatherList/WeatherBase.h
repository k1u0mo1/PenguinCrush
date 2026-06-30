
/**
 * @file   WeatherBase.h
 * @brief  天候の親クラス
 * @author 國田知睦
 * @date   2026/06/17
 */

#pragma once
#include "pch.h"
#include <d3d11.h>
#include <SimpleMath.h>
#include <vector>

/// <summary>
/// 天候エフェクト（雨や雪など）の基底クラス
/// </summary>
class WeatherBase
{
private:

	//横幅の範囲
	static constexpr int SPAWN_RANGE_X = 2000;
	//高さの範囲
	static constexpr int SPAWN_RANGE_Y = 1000;
	//奥行き範囲
	static constexpr int SPAWN_RANGE_Z = 2000;

	//パーティクルの最大数
	static constexpr int MAX_PARTICLES = 5000;

	//パーティクルの初期サイズ
	static constexpr float DEFAULT_PARTICLE_SIZE = 1.0f;

public:

	/// <summary>
	/// 天候用仮想デストラクタ
	/// </summary>
	virtual ~WeatherBase() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="device">Direct3Dのデバイス</param>
	virtual void Initialize(ID3D11Device* device);

	/// <summary>
	/// 更新
	/// </summary>
	/// <param name="elapsedTime">前フレームからの経過時間</param>
	virtual void Update(float elapsedTime);

	/// <summary>
	/// 描画
	/// </summary>
	/// <param name="context">デバイスコンテキスト</param>
	/// <param name="view">ビュー行列<</param>
	/// <param name="proj">射影行列</param>
	/// <param name="camPos">カメラの座標</param>
	virtual void Render(
		ID3D11DeviceContext* context,
		const DirectX::SimpleMath::Matrix& view, 
		const DirectX::SimpleMath::Matrix& proj, 
		const DirectX::SimpleMath::Vector3& camPos) = 0;

protected:

	//共通のリソース
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_constBuffer;

	std::unique_ptr<DirectX::CommonStates> m_states;

	//時間
	float m_time = 0.0f;

	//定数バッファの構造体
	struct  CBWeather
	{
		DirectX::SimpleMath::Matrix view;
		DirectX::SimpleMath::Matrix proj;
		DirectX::SimpleMath::Vector4 time;
		DirectX::SimpleMath::Vector4 cameraPos;
		DirectX::SimpleMath::Vector4 params;
	};
};

