
//DisplayCollision.cpp

#include "pch.h"
#include "DisplayCollision.h"

#ifdef _COLLISION_LINE_ON
#include "DebugDraw"
#endif

using namespace DirectX;

//-----------------------------------------------------------------
// コンストラクタ
//-----------------------------------------------------------------

DisplayCollision::DisplayCollision(
	ID3D11Device* device, 
	ID3D11DeviceContext* context, 
	bool modelActive, bool lineActive,
	uint32_t collisionMax)
	: m_modelActive(modelActive)
	, m_lineActive(lineActive)
	, m_collisionMax(collisionMax)
{
	//モデル/////////////////////////////
	
	//モデルの作成 (球)
	m_modelSphere = GeometricPrimitive::CreateSphere(context, 2.0f, 8);

	//モデルの作成 (ボックス)　
	m_modelBox = GeometricPrimitive::CreateCube(context);

	//エフェクト//////////////////////////////

	//エフェクトの作成（モデル用）
	m_modelEffect = std::make_unique<NormalMapEffect>(device);
	m_modelEffect->SetVertexColorEnabled(false);
	m_modelEffect->SetBiasedVertexNormals(false);
	m_modelEffect->SetInstancingEnabled(true);
	m_modelEffect->SetFogEnabled(false);
	m_modelEffect->SetTexture(nullptr);
	m_modelEffect->DisableSpecular();
	m_modelEffect->EnableDefaultLighting();
	m_modelEffect->SetWorld(SimpleMath::Matrix::Identity);

	// エフェクトの作成（ライン用）
	m_lineEffect = std::make_unique<BasicEffect>(device);
	m_lineEffect->SetVertexColorEnabled(true);
	m_lineEffect->SetTextureEnabled(false);
	m_lineEffect->SetLightingEnabled(false);
	m_lineEffect->SetWorld(SimpleMath::Matrix::Identity);

	//入力レイアウト///////////////////////////////////

	// 入力レイアウトの作成（モデル用）
	const D3D11_INPUT_ELEMENT_DESC c_InputElements[] =
	{
		{ "SV_Position", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA,   0 },
		{ "NORMAL",      0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA,   0 },
		{ "TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA,   0 },
		{ "InstMatrix",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "InstMatrix",  1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "InstMatrix",  2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	};
	DX::ThrowIfFailed(
		CreateInputLayoutFromEffect(device, m_modelEffect.get(),
			c_InputElements, std::size(c_InputElements),
			m_modelInputLayout.ReleaseAndGetAddressOf())
	);

	//定数バッファ/////////////////////////////////////

	// インスタンシング用のワールド行列格納用定数バッファを作成
	auto desc = CD3D11_BUFFER_DESC(
		static_cast<UINT>(DISPLAY_COLLISION_MAX * sizeof(XMFLOAT3X4)),
		D3D11_BIND_VERTEX_BUFFER,
		D3D11_USAGE_DYNAMIC,
		D3D11_CPU_ACCESS_WRITE);
	DX::ThrowIfFailed(
		device->CreateBuffer(&desc, nullptr,
			m_instancedVB.ReleaseAndGetAddressOf())
	);

#ifdef _COLLISION_LINE_ON
	// プリミティブバッチの作成
	m_primitiveBatch = std::make_unique<PrimitiveBatch<VertexPositionColor>>(context);

	// 入力レイアウトの作成
	DX::ThrowIfFailed(
		CreateInputLayoutFromEffect<VertexPositionColor>(device, m_lineEffect.get(),
			m_lineInputLayout.ReleaseAndGetAddressOf())
	);
#endif


}

//-----------------------------------------------------------------
// コリジョンモデルの描画関数
//-----------------------------------------------------------------

void DisplayCollision::DrawCollisionModel(
	ID3D11DeviceContext* context,
	DirectX::CommonStates* states,
	const DirectX::SimpleMath::Matrix& view,
	const DirectX::SimpleMath::Matrix& proj,
	DirectX::FXMVECTOR color)
{
	//登録数が最大表示数を超えていないか？確認
	assert(static_cast<uint32_t>(m_spheres.size()) < m_collisionMax);
	assert(static_cast<uint32_t>(m_boxes.size()) < m_collisionMax);

	//球のモデルを描画//////////////////////////

	//エフェクトを適応する
	m_modelEffect->SetColorAndAlpha(color);
	m_modelEffect->SetView(view);
	m_modelEffect->SetProjection(proj);
	m_modelEffect->Apply(context);

	{
		//ワールド行列を設定
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		context->Map(m_instancedVB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		XMFLOAT3X4* p = static_cast<XMFLOAT3X4*>(mappedResource.pData);
		ZeroMemory(p, sizeof(XMFLOAT3X4) * m_spheres.size());
		for (int i = 0; i < m_spheres.size(); i++)
		{
			//拡大縮小
			p[i]._11 = p[i]._22 = p[i]._33 = m_spheres[i].radius;
			// 位置
			p[i]._14 = m_spheres[i].center.x;
			p[i]._24 = m_spheres[i].center.y;
			p[i]._34 = m_spheres[i].center.z;
		}
		context->Unmap(m_instancedVB.Get(), 0);
	}

	//球をインスタンスし描画している（高速化のため）
	m_modelSphere->DrawInstanced(m_modelEffect.get(), m_modelInputLayout.Get(), static_cast<uint32_t>(m_spheres.size()), true, false, 0, [=]()
		{
			UINT stride = sizeof(XMFLOAT3X4);
			UINT offset = 0;
			context->OMSetDepthStencilState(states->DepthRead(), 0);
			context->IASetVertexBuffers(1, 1, m_instancedVB.GetAddressOf(), &stride, &offset);
		}
	);

	//ボックスのモデルを描画////////////////////////

	{
		//ワールド行列を設定
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		context->Map(m_instancedVB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		XMFLOAT3X4* p = static_cast<XMFLOAT3X4*>(mappedResource.pData);
		ZeroMemory(p, sizeof(XMFLOAT3X4) * m_boxes.size());
		for (int i = 0; i < m_boxes.size(); i++)
		{
			SimpleMath::Matrix s = SimpleMath::Matrix::CreateScale(m_boxes[i].extents * 2.0f);
			SimpleMath::Matrix r = SimpleMath::Matrix::CreateFromQuaternion(m_boxes[i].rotate);
			SimpleMath::Matrix c = SimpleMath::Matrix::CreateTranslation(m_boxes[i].center);
			SimpleMath::Matrix m = s * r * c;
			p[i]._11 = m._11;  p[i]._12 = m._21; p[i]._13 = m._31; p[i]._14 = m._41;
			p[i]._21 = m._12;  p[i]._22 = m._22; p[i]._23 = m._32; p[i]._24 = m._42;
			p[i]._31 = m._13;  p[i]._32 = m._23; p[i]._33 = m._33; p[i]._34 = m._43;
		}
		context->Unmap(m_instancedVB.Get(), 0);
	}

	//ボックスをインスタンスし描画している（高速化のため）
	m_modelBox->DrawInstanced(m_modelEffect.get(), m_modelInputLayout.Get(), static_cast<uint32_t>(m_boxes.size()), true, false, 0, [=]()
		{
			UINT stride = sizeof(XMFLOAT3X4);
			UINT offset = 0;
			context->OMSetDepthStencilState(states->DepthRead(), 0);
			context->IASetVertexBuffers(1, 1, m_instancedVB.GetAddressOf(), &stride, &offset);
		}
	);

	
}

#ifdef _COLLISION_LINE_ON
// コリジョンラインの描画
void DisplayCollision::DrawCollisionLine(ID3D11DeviceContext* context, DirectX::CommonStates* states, const DirectX::SimpleMath::Matrix& view, const DirectX::SimpleMath::Matrix& proj, DirectX::FXMVECTOR color)
{
	// ----- ラインを描画 ----- //

	context->OMSetBlendState(states->Opaque(), nullptr, 0xFFFFFFFF);
	context->OMSetDepthStencilState(states->DepthRead(), 0);
	context->RSSetState(states->CullNone());

	//エフェクトを適応する
	m_lineEffect->SetView(view);
	m_lineEffect->SetProjection(proj);
	m_lineEffect->Apply(context);

	//入力レイアウトを設定する
	context->IASetInputLayout(m_lineInputLayout.Get());

	m_primitiveBatch->Begin();

	//球のラインを描画////////////////////
	for (int i = 0; i < m_spheres.size(); i++)
	{
		DirectX::BoundingSphere shpere(m_spheres[i].center, m_spheres[i].radius);
		SimpleMath::Color lineColor = color;
		if (m_spheres[i].lineColor.w != 0.0f) lineColor = m_spheres[i].lineColor;
		DX::Draw(m_primitiveBatch.get(), shpere, lineColor);
	}

	//ボックスのラインを描画//////////
	for (int i = 0; i < m_boxes.size(); i++)
	{
		DirectX::BoundingOrientedBox box(m_boxes[i].center, m_boxes[i].extents, m_boxes[i].rotate);
		SimpleMath::Color lineColor = color;
		if (m_boxes[i].lineColor.w != 0.0f) lineColor = m_boxes[i].lineColor;
		DX::Draw(m_primitiveBatch.get(), box, lineColor);
	}

	m_primitiveBatch->End();
}

#endif


//-----------------------------------------------------------------
// 登録されたコリジョンの描画関数
//-----------------------------------------------------------------

void DisplayCollision::DrawCollision(
	ID3D11DeviceContext* context, 
	DirectX::CommonStates* states,
	const DirectX::SimpleMath::Matrix& view,
	const DirectX::SimpleMath::Matrix& proj,
	DirectX::FXMVECTOR baseColor,
	DirectX::FXMVECTOR lineColor, 
	float alpha)
{
	//表示フラグがオフなら終了
	if (!m_isVisible)
	{
		m_spheres.clear();
		m_boxes.clear();
		return;
	}

	//色＋アルファ値
	SimpleMath::Color color = baseColor;
	color.w = alpha;

	//コリジョンモデルの描画
	if (m_modelActive) DrawCollisionModel(context, states, view, proj, color);

#ifdef _COLLISION_LINE_ON
	//ラインの色を指定している場合
	SimpleMath::Color c = lineColor;
	if (c.w != 0.0f) color = lineColor;

	//コリジョンラインの描画
	if (m_lineActive) DrawCollisionLine(context, states, view, proj, color);
#else
	UNREFERENCED_PARAMETER(lineColor);
#endif

	//登録された表示情報をクリアする
	m_spheres.clear();
	m_boxes.clear();
}
