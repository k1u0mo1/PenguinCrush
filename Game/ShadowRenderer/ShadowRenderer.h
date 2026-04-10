
//ゲーム全体の影　丸影

#pragma once
#include "pch.h"

#include <d3d11.h>
#include <wrl.h>
#include <DirectXMath.h>
#include "CommonStates.h"
#include "Effects.h"
#include "VertexTypes.h"
#include <memory>

/// <summary>
/// ゲーム全体のキャラクターやオブジェクトの丸影を描画するクラス
/// </summary>
class ShadowRenderer
{
public:

	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="device">Direct3Dのデバイス</param>
	/// <param name="context">Direct3Dのデバイスコンテキスト</param>
	ShadowRenderer(ID3D11Device* device,ID3D11DeviceContext* context);

	/// <summary>
	/// デストラクタ
	/// </summary>
	~ShadowRenderer() = default;

	/// <summary>
	/// 指定した座標に丸影を描画
	/// </summary>
	/// <param name="context">デバイスコンテキスト</param>
	/// <param name="states">コモンステート</param>
	/// <param name="view">ビュー行列</param>
	/// <param name="proj">射影行列</param>
	/// <param name="position">影を描画するワールド座標</param>
	/// <param name="scale">影の大きさ</param>
	/// <param name="rotateX">X軸の回転角度</param>
	/// <param name="rotateZ">Z軸の回転角度</param>
	void Render(
		ID3D11DeviceContext* context,
		DirectX::CommonStates* states,
		const DirectX::SimpleMath::Matrix& view,
		const DirectX::SimpleMath::Matrix& proj,
		const DirectX::SimpleMath::Vector3& position,
		float scale = 1.0f,
		float rotateX = 0.0f,
		float rotateZ = 0.0f
	);

private:

	//板ポリゴン用
	//頂点バッファ
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;

	//インデックスバッファ
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;

	//影のテクスチャ
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_textureSRV;

	//描画用ステートとエフェクト
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_shadowState;
	std::unique_ptr<DirectX::BasicEffect> m_shadowEffect;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_shadowInputLayout;

};