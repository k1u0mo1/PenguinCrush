
/**
 * @file   Particle.cpp
 * @brief  シェーダ用エフェクトのパーティクル管理を行うクラス
 * @author 國田知睦
 * @date   2026/07/29
 */

#include "pch.h"
#include "Particle.h"
#include <Library/BinaryFile.h>
#include "Game/Common/DeviceResources.h"
#include <SimpleMath.h>
#include <Effects.h>
#include <PrimitiveBatch.h>
#include <VertexTypes.h>
#include <WICTextureLoader.h>
#include <CommonStates.h>
#include <vector>
#include <algorithm>

#include "Game/Effects/ParticleList/SplashEmitter.h"
#include "Game/Effects/ParticleList/ExplosionEmitter.h"
#include "Game/Effects/ParticleList/DashEmitter.h"
#include "Game/Effects/ParticleList/HealEmitter.h"

//----------------------------------------------------------
// 頂点シェーダへ渡す頂点データの入力レイアウト定義
//----------------------------------------------------------

const std::vector<D3D11_INPUT_ELEMENT_DESC> Particle::INPUT_LAYOUT =
{
	{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR",	0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, sizeof(DirectX::SimpleMath::Vector3), D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(DirectX::SimpleMath::Vector3) + sizeof(DirectX::SimpleMath::Vector4), D3D11_INPUT_PER_VERTEX_DATA, 0 },
};

//----------------------------------------------------------
// コンストラクタ
//----------------------------------------------------------

Particle::Particle()
	: 
	m_pDR(nullptr)
{
	//初期化時に生成ルールを辞書に登録
	//水しぶき
	m_creatorMap[Type::Splash] = [](auto pos, auto count) {
		return std::make_unique<SplashEmitter>(pos, count);
		};
	//爆発
	m_creatorMap[Type::Explosion] = [](auto pos, auto count) {
		return std::make_unique<ExplosionEmitter>(pos, count);
		};
	//ダッシュの煙
	m_creatorMap[Type::Dash] = [](auto pos, auto count) {
		return std::make_unique<DashEmitter>(pos, count);
		};
	//回復
	m_creatorMap[Type::Heal] = [](auto pos, auto count) {
		return std::make_unique<HealEmitter>(pos, count);
		};
}

//----------------------------------------------------------
// デストラクタ
//----------------------------------------------------------

Particle::~Particle()
{
}

//----------------------------------------------------------
// 演出に使用するテクスチャを読み込む
//----------------------------------------------------------

void Particle::LoadTexture(const wchar_t* path)
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
// デバイスリソースの初期化
//----------------------------------------------------------

void Particle::Initialize(DX::DeviceResources* pDR)
{
	m_pDR = pDR;
	ID3D11Device1* device = pDR->GetD3DDevice();

	//シェーダーの作成
	CreateShader();

	//画像の読み込み まだない
	LoadTexture(L"Resources/Textures/White.png");  //０番目画像
	LoadTexture(L"Resources/Textures/Heal.png");   //１番目画像
	
	//プリミティブバッチの作成
	m_batch = std::make_unique<DirectX::PrimitiveBatch<DirectX::VertexPositionColorTexture>>(pDR->GetD3DDeviceContext());

	m_states = std::make_unique<DirectX::CommonStates>(device);
}

//----------------------------------------------------------
// パーティクルの移動・寿命計算などの更新処理
//----------------------------------------------------------

void Particle::Update(float elapsedTime)
{
	//全てのエミッターを更新
	for (auto& emitter : m_emitters)
	{
		emitter->Update(elapsedTime);
	}
	//終了したエミッターを管理者から削除
	m_emitters.erase(
		std::remove_if(m_emitters.begin(), m_emitters.end(),
			[](const std::unique_ptr<BaseParticleEmitter>& e) {return e->Isdead(); }),
		m_emitters.end());
}

//----------------------------------------------------------
// 指定した座標にパーティクルを発生
//----------------------------------------------------------

void Particle::Spawn(
	Type type,
	const DirectX::SimpleMath::Vector3& position,
	int count,
	float size)
{
	UNREFERENCED_PARAMETER(size);

	//辞書にTypeが登録されているかを確認して、リストに追加する
	if (m_creatorMap.count(type))
	{
		m_emitters.push_back(m_creatorMap[type](position, count));
	}
}

//----------------------------------------------------------
// 3D空間へのパーティクル描画処理
//----------------------------------------------------------

void Particle::Render(DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix proj)
{

	ID3D11DeviceContext1* context = m_pDR->GetD3DDeviceContext();

	//頂点情報(板ポリゴンの４頂点の座標情報）
	DirectX::VertexPositionColorTexture vertex[4] =
	{
		DirectX::VertexPositionColorTexture(DirectX::SimpleMath::Vector3(0.0f,  0.0f, 0.0f),DirectX::SimpleMath::Vector4::One,DirectX::SimpleMath::Vector2(0.0f, 0.0f)),
	};

	//シェーダーに渡す追加のバッファを作成する。(ConstBuffer）
	ConstBuffer cbuff;
	cbuff.matView = view.Transpose();
	cbuff.matProj = proj.Transpose();
	cbuff.matWorld = DirectX::SimpleMath::Matrix::Identity;
	cbuff.Diffuse = DirectX::SimpleMath::Vector4(1, 1, 1, 1);
	
	//受け渡し用バッファの内容更新(ConstBufferからID3D11Bufferへの変換）
	context->UpdateSubresource(m_CBuffer.Get(), 0, NULL, &cbuff, 0, 0);

	//シェーダーにバッファを渡す
	ID3D11Buffer* cb[1] = { m_CBuffer.Get() };
	context->VSSetConstantBuffers(0, 1, cb);
	context->GSSetConstantBuffers(0, 1, cb);
	context->PSSetConstantBuffers(0, 1, cb);

	//画像用サンプラーの登録
	ID3D11SamplerState* sampler[1] = { m_states->LinearWrap() };
	context->PSSetSamplers(0, 1, sampler);

	//半透明描画指定
	ID3D11BlendState* blendstate = m_states->NonPremultiplied();

	//透明判定処理
	context->OMSetBlendState(blendstate, nullptr, 0xFFFFFFFF);

	//深度バッファに書き込み参照する
	context->OMSetDepthStencilState(m_states->DepthRead(), 0);

	//カリングは左周り
	context->RSSetState(m_states->CullNone());

	//シェーダをセットする
	context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	context->GSSetShader(m_geometryShader.Get(), nullptr, 0);
	context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

	////ピクセルシェーダにテクスチャを登録する。
	//for (int i = 0; i < m_texture.size(); i++)
	//{
	//	context->PSSetShaderResources(i, 1, m_texture[i].GetAddressOf());
	//}

	//インプットレイアウトの登録
	context->IASetInputLayout(m_inputLayout.Get());

	//板ポリゴンを描画
	//m_batch->Begin();
	
	//エミッターごとに画像をセットし描画をする
	for (auto& emitter : m_emitters)
	{
		//指定したテクスチャの番号を取得
		int texIndex = emitter->GetTextureUndex();

		if (texIndex >= 0 && texIndex < static_cast<int>(m_texture.size()))
		{
			context->PSSetShaderResources(0, 1, m_texture[texIndex].GetAddressOf());
		}

		m_batch->Begin();
		emitter->Render(m_batch.get());
		m_batch->End();
	}

	//m_batch->End();

	//	シェーダの登録を解除しておく
	context->VSSetShader(nullptr, nullptr, 0);
	context->GSSetShader(nullptr, nullptr, 0);
	context->PSSetShader(nullptr, nullptr, 0);
}

//----------------------------------------------------------
// パーティクル描画用のシェーダとインプットレイアウトを作成
//----------------------------------------------------------

void Particle::CreateShader()
{
	ID3D11Device1* device = m_pDR->GetD3DDevice();

	//	コンパイルされたシェーダファイルを読み込み
	BinaryFile VSData = BinaryFile::LoadFile(L"Resources/Shaders/ParticleVS.cso");
	BinaryFile GSData = BinaryFile::LoadFile(L"Resources/Shaders/ParticleGS.cso");
	BinaryFile PSData = BinaryFile::LoadFile(L"Resources/Shaders/ParticlePS.cso");

	//インプットレイアウトの作成
	device->CreateInputLayout(&INPUT_LAYOUT[0],
		static_cast<UINT>(INPUT_LAYOUT.size()),
		VSData.GetData(), VSData.GetSize(),
		m_inputLayout.GetAddressOf());

	//頂点シェーダ作成
	if (FAILED(device->CreateVertexShader(VSData.GetData(), VSData.GetSize(), NULL, m_vertexShader.ReleaseAndGetAddressOf())))
	{
		//エラー
		MessageBox(0, L"CreateVertexShader Failed.", NULL, MB_OK);
		return;
	}

	//ジオメトリシェーダ作成
	if (FAILED(device->CreateGeometryShader(GSData.GetData(), GSData.GetSize(), NULL, m_geometryShader.ReleaseAndGetAddressOf())))
	{
		//エラー
		MessageBox(0, L"CreateGeometryShader Failed.", NULL, MB_OK);
		return;
	}
	//ピクセルシェーダ作成
	if (FAILED(device->CreatePixelShader(PSData.GetData(), PSData.GetSize(), NULL, m_pixelShader.ReleaseAndGetAddressOf())))
	{
		//エラー
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
