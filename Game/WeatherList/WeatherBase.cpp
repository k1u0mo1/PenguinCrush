
/**
 * @file   WeatherBase.cpp
 * @brief  天候の親クラス
 * @author 國田知睦
 * @date   2026/06/17
 */

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

	std::vector<Vertex> vertices(MAX_PARTICLES);

	for (int i = 0; i < MAX_PARTICLES; i++)
	{
		vertices[i].Pos = DirectX::SimpleMath::Vector3(
			(rand() % SPAWN_RANGE_X) - (SPAWN_RANGE_X * 0.5f),
			(rand() % SPAWN_RANGE_Y) / 10.0f,
			(rand() % SPAWN_RANGE_Z) - (SPAWN_RANGE_Z * 0.5f)
		);

		//サイズ
		vertices[i].Size = 
			DirectX::SimpleMath::Vector2(DEFAULT_PARTICLE_SIZE, DEFAULT_PARTICLE_SIZE);
	}

	//頂点バッファの作成
	D3D11_BUFFER_DESC bd = { sizeof(Vertex) * MAX_PARTICLES, D3D11_USAGE_IMMUTABLE, D3D11_BIND_VERTEX_BUFFER, 0, 0, 0 };
	
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