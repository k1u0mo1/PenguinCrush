
//Smoke


#include "pch.h"
#include "Smoke.h"

#include "Library/BinaryFile.h"
#include "Game/Common/DeviceResources.h"

#include <SimpleMath.h>
#include <Effects.h>
#include <PrimitiveBatch.h>
#include <VertexTypes.h>
#include <WICTextureLoader.h>
#include <CommonStates.h>
#include <vector>

using namespace DirectX;


//----------------------------------------------------------
// 頂点シェーダへ渡す頂点データの入力レイアウト定義
//----------------------------------------------------------

const std::vector<D3D11_INPUT_ELEMENT_DESC> Smoke::INPUT_LAYOUT =
{
	{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR",	0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, sizeof(SimpleMath::Vector3), D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(SimpleMath::Vector3) + sizeof(SimpleMath::Vector4), D3D11_INPUT_PER_VERTEX_DATA, 0 },
};

//----------------------------------------------------------
// コンストラクタ
//----------------------------------------------------------

Smoke::Smoke()
	: m_pDR(nullptr)
{
}

//----------------------------------------------------------
// デストラクタ
//----------------------------------------------------------

Smoke::~Smoke()
{
}

//----------------------------------------------------------
// 煙のテクスチャを読み込む
//----------------------------------------------------------

void Smoke::LoadTexture(const wchar_t* path)
{
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture;
	
	DirectX::CreateWICTextureFromFile(
		m_pDR->GetD3DDevice(), 
		path,
		nullptr, 
		texture.ReleaseAndGetAddressOf());

	m_texture.push_back(texture);
}

//----------------------------------------------------------
// 初期化処理
//----------------------------------------------------------

void Smoke::Initialize(DX::DeviceResources* pDR)
{
	m_pDR = pDR;
	ID3D11Device1* device = pDR->GetD3DDevice();

	//	シェーダーの作成
	CreateShader();

	//	画像の読み込み まだない
	LoadTexture(L"Resources/Textures/White.png");
	//LoadTexture(L"Resources/Textures/floor.png");

	//	プリミティブバッチの作成
	m_batch = std::make_unique<PrimitiveBatch<VertexPositionColorTexture>>(pDR->GetD3DDeviceContext());

	m_states = std::make_unique<CommonStates>(device);

}

//----------------------------------------------------------
// 煙の描画
//----------------------------------------------------------

void Smoke::Render(
	DirectX::SimpleMath::Matrix view,
	DirectX::SimpleMath::Matrix proj,
	const DirectX::SimpleMath::Vector3& position,
	float scale,
	float alpha
)
{

	ID3D11DeviceContext1* context = m_pDR->GetD3DDeviceContext();

	//	頂点情報(板ポリゴンの４頂点の座標情報）
	VertexPositionColorTexture vertex[4] =
	{
		VertexPositionColorTexture(
			SimpleMath::Vector3(0.0f,  0.0f, 0.0f),
			SimpleMath::Vector4(1.0f,1.0f,1.0f,alpha),
			SimpleMath::Vector2(0.0f, 0.0f)),
	};

	SimpleMath::Matrix matScale = SimpleMath::Matrix::CreateScale(scale);
	SimpleMath::Matrix matRot = SimpleMath::Matrix::CreateRotationX(DirectX::XM_PIDIV2);
	SimpleMath::Matrix matTrans = SimpleMath::Matrix::CreateTranslation(position);
	SimpleMath::Matrix matWorld = matScale * matRot * matTrans;

	ConstBuffer cbuff;
	
	cbuff.matWorld = matWorld.Transpose();
	cbuff.matView = view.Transpose();
	cbuff.matProj = proj.Transpose();


	////	シェーダーに渡す追加のバッファを作成する。(ConstBuffer）
	//ConstBuffer cbuff;
	//cbuff.matView = view.Transpose();
	//cbuff.matProj = proj.Transpose();
	//cbuff.matWorld = SimpleMath::Matrix::Identity;
	//cbuff.Diffuse = SimpleMath::Vector4(1, 1, 1, 1);
	
	cbuff.time = SimpleMath::Vector4(float(m_timer.GetTotalSeconds()), 0, 0, 0);
	
	//	受け渡し用バッファの内容更新(ConstBufferからID3D11Bufferへの変換）
	context->UpdateSubresource(m_CBuffer.Get(), 0, NULL, &cbuff, 0, 0);

	//	シェーダーにバッファを渡す
	ID3D11Buffer* cb[1] = { m_CBuffer.Get() };
	context->VSSetConstantBuffers(0, 1, cb);
	context->GSSetConstantBuffers(0, 1, cb);
	context->PSSetConstantBuffers(0, 1, cb);

	//	画像用サンプラーの登録
	ID3D11SamplerState* sampler[1] = { m_states->LinearWrap() };
	context->PSSetSamplers(0, 1, sampler);

	//	半透明描画指定
	ID3D11BlendState* blendstate = m_states->NonPremultiplied();

	//	透明判定処理
	context->OMSetBlendState(blendstate, nullptr, 0xFFFFFFFF);

	//	深度バッファに書き込み参照する
	//context->OMSetDepthStencilState(m_states->DepthDefault(), 0);
	//context->OMSetDepthStencilState(m_states->DepthRead(), 0);
	context->OMSetDepthStencilState(m_states->DepthNone(), 0);

	//	カリングは左周り
	context->RSSetState(m_states->CullNone());

	//	シェーダをセットする
	context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	context->GSSetShader(m_geometryShader.Get(), nullptr, 0);
	context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

	//	ピクセルシェーダにテクスチャを登録する。
	for (int i = 0; i < m_texture.size(); i++)
	{
		context->PSSetShaderResources(i, 1, m_texture[i].GetAddressOf());
	}

	//	インプットレイアウトの登録
	context->IASetInputLayout(m_inputLayout.Get());

	//	板ポリゴンを描画
	m_batch->Begin();
	m_batch->Draw(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST, &vertex[0], 1);
	m_batch->End();

	//	シェーダの登録を解除しておく
	context->VSSetShader(nullptr, nullptr, 0);
	context->GSSetShader(nullptr, nullptr, 0);
	context->PSSetShader(nullptr, nullptr, 0);

	context->OMSetDepthStencilState(m_states->DepthDefault(), 0);

}

//----------------------------------------------------------
// 煙描画用のシェーダを作成
//----------------------------------------------------------

void Smoke::CreateShader()
{
	ID3D11Device1* device = m_pDR->GetD3DDevice();

	//	コンパイルされたシェーダファイルを読み込み
	BinaryFile VSData = BinaryFile::LoadFile(L"Resources/Shaders/SmokeVS.cso");
	BinaryFile GSData = BinaryFile::LoadFile(L"Resources/Shaders/SmokeGS.cso");
	BinaryFile PSData = BinaryFile::LoadFile(L"Resources/Shaders/SmokePS.cso");

	//	インプットレイアウトの作成
	device->CreateInputLayout(&INPUT_LAYOUT[0],
		static_cast<UINT>(INPUT_LAYOUT.size()),
		VSData.GetData(), VSData.GetSize(),
		m_inputLayout.GetAddressOf());

	//	頂点シェーダ作成
	if (FAILED(device->CreateVertexShader(VSData.GetData(), VSData.GetSize(), NULL, m_vertexShader.ReleaseAndGetAddressOf())))
	{//	エラー
		MessageBox(0, L"CreateVertexShader Failed.", NULL, MB_OK);
		return;
	}

	//	ジオメトリシェーダ作成
	if (FAILED(device->CreateGeometryShader(GSData.GetData(), GSData.GetSize(), NULL, m_geometryShader.ReleaseAndGetAddressOf())))
	{//	エラー
		MessageBox(0, L"CreateGeometryShader Failed.", NULL, MB_OK);
		return;
	}
	//	ピクセルシェーダ作成
	if (FAILED(device->CreatePixelShader(PSData.GetData(), PSData.GetSize(), NULL, m_pixelShader.ReleaseAndGetAddressOf())))
	{//	エラー
		MessageBox(0, L"CreatePixelShader Failed.", NULL, MB_OK);
		return;
	}

	//	シェーダーにデータを渡すためのコンスタントバッファ生成
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	device->CreateBuffer(&bd, nullptr, &m_CBuffer);
}
