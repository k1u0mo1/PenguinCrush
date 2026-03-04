
//ゲーム全体の影

#include "pch.h"
#include "ShadowRenderer.h"

//テクスチャの読み込み用
#include <DDSTextureLoader.h>

using namespace DirectX;

//板ポリゴンの頂点データ構造体
struct VertexData
{
	SimpleMath::Vector3 position;
	//SimpleMath::Vector3 normal;
	SimpleMath::Vector2 texCoord;
};

ShadowRenderer::ShadowRenderer(
	ID3D11Device* device,
	ID3D11DeviceContext* /*context*/
)
{
	//-------------------------------------------------------
	//影用テクスチャの読み込み 
	//-------------------------------------------------------

	CreateDDSTextureFromFile(
		device,
		L"Resources/Textures/Shadow.dds",
		nullptr,
		m_textureSRV.GetAddressOf()
	);

	//-------------------------------------------------------
	//板ポリゴンの作成
	//-------------------------------------------------------
	VertexData vertices[] =
	{
		{ SimpleMath::Vector3(-0.5f, 0.0f,  0.5f), SimpleMath::Vector2(0.0f, 0.0f) },
		{ SimpleMath::Vector3(0.5f, 0.0f,  0.5f), SimpleMath::Vector2(1.0f, 0.0f) },
		{ SimpleMath::Vector3(-0.5f, 0.0f, -0.5f), SimpleMath::Vector2(0.0f, 1.0f) },
		{ SimpleMath::Vector3(0.5f, 0.0f, -0.5f), SimpleMath::Vector2(1.0f, 1.0f) },
	};

	//頂点バッファの作成
	D3D11_BUFFER_DESC vbd = {};
	vbd.Usage = D3D11_USAGE_DEFAULT;
	vbd.ByteWidth = sizeof(VertexData) * 4;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	D3D11_SUBRESOURCE_DATA viniData = {};
	viniData.pSysMem = vertices;
	device->CreateBuffer(&vbd, &viniData, m_vertexBuffer.GetAddressOf());

	//インデックスデータ
	unsigned short indices[] =
	{
		0,1,2,
		1,3,2
	};

	//インデックスバッファの作成
	D3D11_BUFFER_DESC ibd = {};
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.ByteWidth = sizeof(unsigned short) * 6;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	D3D11_SUBRESOURCE_DATA iinitData = {};
	iinitData.pSysMem = indices;
	device->CreateBuffer(&ibd, &iinitData, m_indexBuffer.GetAddressOf());

	//-------------------------------------------------------
	//ステートとエフェクトの初期化
	//-------------------------------------------------------
	//影用ラスタライザステート
	D3D11_RASTERIZER_DESC rd = {};
	rd.FillMode = D3D11_FILL_SOLID;
	rd.CullMode = D3D11_CULL_NONE; 
	rd.DepthClipEnable = TRUE;

	//チラつきの防止　深度バイアス設定
	rd.DepthBias = -1000;
	rd.SlopeScaledDepthBias = -2.0f;
	rd.DepthBiasClamp = 0.0f;
	device->CreateRasterizerState(&rd, m_shadowState.GetAddressOf());

	//ベーシックエフェクトの初期化
	m_shadowEffect = std::make_unique<BasicEffect>(device);
	//ライティングは不要
	m_shadowEffect->SetLightingEnabled(false); 
	//テクスチャを使用
	m_shadowEffect->SetTextureEnabled(true);  

	//m_shadowEffect->SetDiffuseColor(Colors::Red);
	//m_shadowEffect->SetAlpha(1.0f);
	//// ライティング無効などはそのまま
	//m_shadowEffect->SetLightingEnabled(false);

	//読み込んだテクスチャをセット
	m_shadowEffect->SetTexture(m_textureSRV.Get()); 

	//インプットレイアウトの作成
	void const* shaderByteCode;
	size_t byteCodeLength;
	m_shadowEffect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);

	//
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "SV_Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		/*{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },*/
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	device->CreateInputLayout(
		layout,
		ARRAYSIZE(layout),
		shaderByteCode, byteCodeLength,
		m_shadowInputLayout.GetAddressOf()
	);


}

void ShadowRenderer::Render(
	ID3D11DeviceContext* context, 
	DirectX::CommonStates* states,
	const DirectX::SimpleMath::Matrix& view,
	const DirectX::SimpleMath::Matrix& proj,
	const DirectX::SimpleMath::Vector3& position,
	float scale,
	float rotateX,
	float rotateZ
)
{
	if (!states)return;

	//-------------------------------------------------------
	//影のワールド行列を計算
	//-------------------------------------------------------
	
	//拡大縮小
	SimpleMath::Matrix matScale = SimpleMath::Matrix::CreateScale(scale);

	//回転
	SimpleMath::Matrix matRotX = SimpleMath::Matrix::CreateRotationX(rotateX);
	SimpleMath::Matrix matRotZ = SimpleMath::Matrix::CreateRotationZ(rotateZ);

	//回転を合わせる
	SimpleMath::Matrix matRot = matRotZ * matRotX;

	//垂直方向を計算する
	SimpleMath::Vector3 upVector = SimpleMath::Vector3::Up;
	SimpleMath::Vector3 normal = SimpleMath::Vector3::TransformNormal(upVector, matRot);

	//描画位置 モデルにピッタリつく
	SimpleMath::Vector3 drawPos = position + (normal * 0.02f);

	//平行移動
	SimpleMath::Matrix matTrans = SimpleMath::Matrix::CreateTranslation(drawPos);

	//計算 合体
	SimpleMath::Matrix shadowWorld = matScale * matRot * matTrans;

	//-------------------------------------------------------
	//ステートの保持と設定
	//-------------------------------------------------------
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> oldRasterizerState;
	Microsoft::WRL::ComPtr<ID3D11BlendState> oldBlendState;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> oldDepthState;
	UINT oldSampleMask, oldStencilRef;
	float oldBlendFactor[4];

	context->RSGetState(oldRasterizerState.GetAddressOf());
	context->OMGetBlendState(oldBlendState.GetAddressOf(), oldBlendFactor, &oldSampleMask);
	context->OMGetDepthStencilState(oldDepthState.GetAddressOf(), &oldStencilRef);

	//影用のステートをセット
	context->RSSetState(m_shadowState.Get());
	//半透明の合成を有効
	context->OMSetBlendState(states->AlphaBlend(), nullptr, 0xFFFFFFFF);
	//深度の書き込みを無効
	context->OMSetDepthStencilState(states->DepthRead(), 0);

	//-------------------------------------------------------
	//板ポリゴンの描画
	//-------------------------------------------------------
	//エフェクトに行列をセット
	m_shadowEffect->SetWorld(shadowWorld);
	m_shadowEffect->SetView(view);
	m_shadowEffect->SetProjection(proj);

	//テクスチャがセットされているか確認
	m_shadowEffect->SetTexture(m_textureSRV.Get());

	//エフェクトを適用
	m_shadowEffect->Apply(context);

	//頂点バッファとインデックスバッファ、レイアウトをセットする
	UINT stride = sizeof(VertexData);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
	context->IASetInputLayout(m_shadowInputLayout.Get());
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//描画
	context->DrawIndexed(6, 0, 0);

	//-------------------------------------------------------
	//ステートをもとに戻す
	//-------------------------------------------------------
	context->RSSetState(oldRasterizerState.Get());
	context->OMSetBlendState(oldBlendState.Get(), oldBlendFactor, oldSampleMask);
	context->OMSetDepthStencilState(oldDepthState.Get(), oldStencilRef);

}

