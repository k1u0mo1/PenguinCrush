
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

class ShadowRenderer
{
public:

	//コンストラクタ
	ShadowRenderer(ID3D11Device* device,ID3D11DeviceContext* context);

	//デストラクタ
	~ShadowRenderer() = default;

	//影の描画
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