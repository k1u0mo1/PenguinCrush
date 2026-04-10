
//Particle


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

using namespace DirectX;

//----------------------------------------------------------
// 頂点シェーダへ渡す頂点データの入力レイアウト定義
//----------------------------------------------------------

const std::vector<D3D11_INPUT_ELEMENT_DESC> Particle::INPUT_LAYOUT =
{
	{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR",	0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, sizeof(SimpleMath::Vector3), D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(SimpleMath::Vector3) + sizeof(SimpleMath::Vector4), D3D11_INPUT_PER_VERTEX_DATA, 0 },
};

//----------------------------------------------------------
// コンストラクタ
//----------------------------------------------------------

Particle::Particle()
	: m_pDR(nullptr)
{
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
// ランダムヘルパー (簡易的なもの)
//----------------------------------------------------------

static float GetRandom(float min, float max)
{
	return min + (float)rand() / RAND_MAX * (max - min);
}

//----------------------------------------------------------
// パーティクルの移動・寿命計算などの更新処理
//----------------------------------------------------------

void Particle::Update(float elapsedTime)
{
	// 重力設定 (下向きの力)
	const float gravity = -9.8f * 2.0f; // 少し強めに

	//------------------------------------
	// 水しぶき更新
	//------------------------------------

	for (auto& p : m_splashList)
	{
		// 速度に重力を加算
		p.Velocity.y += gravity * elapsedTime;

		// 位置を更新
		p.Position += p.Velocity * elapsedTime;

		// 年齢を加算
		p.Age += elapsedTime;
	}

	// 寿命切れのパーティクルを削除
	m_splashList.erase(
		std::remove_if(m_splashList.begin(), m_splashList.end(),
			[](const ParticleInfo& p) { return p.Age >= p.Lifetime; }),
		m_splashList.end());

	//------------------------------------
	// 爆発更新
	//------------------------------------

	for (auto& p : m_explosionList)
	{
		// 速度に重力を加算
		p.Velocity.y += gravity * elapsedTime;

		// 位置を更新
		p.Position += p.Velocity * elapsedTime;

		// 年齢を加算
		p.Age += elapsedTime;
	}

	// 寿命切れのパーティクルを削除
	m_explosionList.erase(
		std::remove_if(m_explosionList.begin(), m_explosionList.end(),
			[](const ParticleInfo& p) { return p.Age >= p.Lifetime; }),
		m_explosionList.end());

	//------------------------------------
	// ダッシュ中の更新
	//------------------------------------

	for (auto& p : m_dashList)
	{
		// 速度に重力を加算
		p.Velocity.y += gravity * elapsedTime;

		// 位置を更新
		p.Position += p.Velocity * elapsedTime;

		// 年齢を加算
		p.Age += elapsedTime;
	}

	// 寿命切れのパーティクルを削除
	m_dashList.erase(
		std::remove_if(m_dashList.begin(), m_dashList.end(),
			[](const ParticleInfo& p) { return p.Age >= p.Lifetime; }),
		m_dashList.end());
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
	//count分ループ
	for (int i = 0; i < count; i++)
	{
		//パーティクルの粒のやつ
		ParticleInfo p;
		p.Position = position;
		p.Age = 0.0f;

		p.Size = size;

		//タイプの切り替えでエフェクトを変える
		switch (type)
		{
		//----------------------------------------
		//水しぶき
		//----------------------------------------
		case Type::Splash:
		{
			//色
			p.Color = SimpleMath::Vector4(0.8f, 0.9f, 1.0f, 1.0f);

			// 少しばらつかせる
			p.Position.x += GetRandom(-0.5f, 0.5f);
			p.Position.z += GetRandom(-0.5f, 0.5f);

			// 上方向に飛び散る速度
			float speed = GetRandom(5.0f, 30.0f);
			float angle = GetRandom(0.0f, DirectX::XM_2PI);
			float radius = GetRandom(0.0f, 5.0f); // 横への広がり

			p.Velocity.x = cos(angle) * radius;
			p.Velocity.y = speed; // 上向き
			p.Velocity.z = sin(angle) * radius;

			p.Age = 0.0f;
			p.Lifetime = GetRandom(0.5f, 3.0f);

			m_splashList.push_back(p);
			break;
		}
		//----------------------------------------
		//爆発
		//----------------------------------------
		case Type::Explosion:
		{
			//色
			p.Color = SimpleMath::Vector4(1.0f, 0.5f, 0.2f, 1.0f);
			// 少しばらつかせる
			p.Position.x += GetRandom(-0.5f, 0.5f);
			p.Position.z += GetRandom(-0.5f, 0.5f);

			// 上方向に飛び散る速度
			float speed = GetRandom(5.0f, 10.0f);
			float angle = GetRandom(0.0f, DirectX::XM_2PI);
			float radius = GetRandom(0.0f, 3.0f); // 横への広がり

			p.Velocity.x = cos(angle) * radius;
			p.Velocity.y = speed; 
			p.Velocity.z = sin(angle) * radius;

			p.Age = 0.0f;
			p.Lifetime = GetRandom(0.5f, 3.0f);

			m_explosionList.push_back(p);
			break;
		}
		//----------------------------------------
		//ダッシュ
		//----------------------------------------
		case Type::Dash:
		{
			//色
			p.Color = SimpleMath::Vector4(1.0f, 1.0f, 1.0f, 0.0f);
			// 少しばらつかせる
			p.Position.x += GetRandom(-0.3f, 0.3f);
			p.Position.z += GetRandom(-0.3f, 0.3f);

			// 上方向に飛び散る速度
			float speed = GetRandom(1.0f, 2.0f);
			float angle = GetRandom(0.0f, DirectX::XM_2PI);
			float radius = GetRandom(0.0f, 1.0f); // 横への広がり

			p.Velocity.x = cos(angle) * radius;
			p.Velocity.y = speed;
			p.Velocity.z = sin(angle) * radius;

			p.Age = 0.0f;
			p.Lifetime = GetRandom(0.5f, 3.0f);

			m_dashList.push_back(p);
			break;
		}
		}


		
	}

}

//----------------------------------------------------------
// 3D空間へのパーティクル描画処理
//----------------------------------------------------------

void Particle::Render(DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix proj)
{

	ID3D11DeviceContext1* context = m_pDR->GetD3DDeviceContext();

	//	頂点情報(板ポリゴンの４頂点の座標情報）
	VertexPositionColorTexture vertex[4] =
	{
		VertexPositionColorTexture(SimpleMath::Vector3(0.0f,  0.0f, 0.0f),SimpleMath::Vector4::One,SimpleMath::Vector2(0.0f, 0.0f)),
	};

	//	シェーダーに渡す追加のバッファを作成する。(ConstBuffer）
	ConstBuffer cbuff;
	cbuff.matView = view.Transpose();
	cbuff.matProj = proj.Transpose();
	cbuff.matWorld = SimpleMath::Matrix::Identity;
	cbuff.Diffuse = SimpleMath::Vector4(1, 1, 1, 1);
	
	
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
	context->OMSetDepthStencilState(m_states->DepthDefault(), 0);

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
	/*m_batch->Draw(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST, &vertex[0], 1);*/

	// エフェクト描画用
	auto DrawParticleList = [&](const std::vector<ParticleInfo>& list)
		{
			for (const auto& p : list)
			{
				float ratio = p.Age / p.Lifetime;
				float alpha = 1.0f - ratio;
				if (alpha < 0.0f) alpha = 0.0f;

				DirectX::SimpleMath::Vector4 color = p.Color;
				color.w = alpha;

				VertexPositionColorTexture v(
					p.Position,
					color,
					SimpleMath::Vector2(p.Size, 0.0f) 
				);

				// 描画
				m_batch->Draw(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST, &v, 1);
			}
		};

	// 水しぶきを描画
	DrawParticleList(m_splashList);

	// 爆発を描画
	DrawParticleList(m_explosionList);

	// ダッシュ中を描画
	DrawParticleList(m_dashList);

	m_batch->End();

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
