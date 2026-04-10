

#include "pch.h"
#include "WeatherBase.h"

using namespace DirectX;

//----------------------------------------------------------
// 初期化
//----------------------------------------------------------

void WeatherBase::Initialize(ID3D11Device* device)
{

	//時間の初期化
	m_time = 0.0f;

	//共通ステートの作成
	m_states = std::make_unique<CommonStates>(device);

	//定数バッファの作成
	CD3D11_BUFFER_DESC cbd(sizeof(CBWeather), D3D11_BIND_CONSTANT_BUFFER);
	device->CreateBuffer(&cbd, nullptr, m_constBuffer.ReleaseAndGetAddressOf());

	//頂点データの作成
	struct  Vertex
	{
		SimpleMath::Vector3 Pos;
		SimpleMath::Vector2 Size;
	};

	//最大の数
	int particleCount = 5000;

	std::vector<Vertex> vertices(particleCount);

	for (int i = 0; i < particleCount; i++)
	{
		vertices[i].Pos = DirectX::SimpleMath::Vector3(
			(rand() % 2000 - 1000) / 10.0f,
			(rand() % 1000 ) / 10.0f,
			(rand() % 2000 - 1000) / 10.0f
		);
		//サイズ（仮）
		vertices[i].Size = DirectX::SimpleMath::Vector2(1.0f, 1.0f);
	}

	//頂点バッファの作成
	D3D11_BUFFER_DESC bd = { sizeof(Vertex) * particleCount, D3D11_USAGE_IMMUTABLE, D3D11_BIND_VERTEX_BUFFER, 0, 0, 0 };
	
	D3D11_SUBRESOURCE_DATA data = { vertices.data(), 0, 0 };
	
	device->CreateBuffer(&bd, &data, m_vertexBuffer.ReleaseAndGetAddressOf());
}

//----------------------------------------------------------
// 更新
//----------------------------------------------------------

void WeatherBase::Update(float elapsedTime)
{
	// 時間を進める
	m_time += elapsedTime;
}