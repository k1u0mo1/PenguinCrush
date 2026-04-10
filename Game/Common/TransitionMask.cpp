
//TransitionMask.cpp

#include "pch.h"
#include "TransitionMask.h"
#include "ReadData.h"

using namespace DirectX;

//-----------------------------------------------------------------
// トランジションマスクを初期化
//-----------------------------------------------------------------

TransitionMask::TransitionMask(
	ID3D11Device* device,
	ID3D11DeviceContext* context, 
	float interval)
	: m_interval(interval)
	, m_rate(0.0f)
	, m_open(true)
	, m_request()
{
	//スプライトバッチの作成
	m_spriteBatch = std::make_unique<SpriteBatch>(context);

	{
		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width = 1;
		desc.Height = 1;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;

		uint32_t pixel = 0xFFFFFFFF; // 白 (R,G,B,A = 1,1,1,1)
		D3D11_SUBRESOURCE_DATA initData = {};
		initData.pSysMem = &pixel;
		initData.SysMemPitch = sizeof(uint32_t);

		Microsoft::WRL::ComPtr<ID3D11Texture2D> tex;
		device->CreateTexture2D(&desc, &initData, tex.GetAddressOf());
		device->CreateShaderResourceView(tex.Get(), nullptr, m_dummyTexture.ReleaseAndGetAddressOf());
	}

	//ピクセルシェーダ
	std::vector<uint8_t> fade_ps = DX::ReadData(L"Resources/Shaders/FadePS.cso");

	DX::ThrowIfFailed(
		device->CreatePixelShader(
			fade_ps.data(), 
			fade_ps.size(), 
			nullptr,
			m_fade_PS.ReleaseAndGetAddressOf())
	);
	 
	//マスク用テクスチャの作成
	DX::ThrowIfFailed(
		CreateDDSTextureFromFile
		(
			device,
			L"Resources/Textures/FadeMask.dds",
			nullptr,
			m_maskTexture.ReleaseAndGetAddressOf()
		)
	);

	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.ByteWidth = static_cast<UINT>(sizeof(ConstantBuffer));
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	DX::ThrowIfFailed(device->CreateBuffer(&bufferDesc, nullptr, m_constantBuffer.ReleaseAndGetAddressOf()));

}

//-----------------------------------------------------------------
// トランジションの進行度を更新
//-----------------------------------------------------------------

void TransitionMask::Update(float elapsedTime)
{
	if (m_open)
	{
		// オープン
		m_rate -= elapsedTime / m_interval;
		if (m_rate < 0.0f)
		{
			m_rate = 0.0f;
		}

	}
	else
	{
		// クローズ
		m_rate += elapsedTime / m_interval;
		if (m_rate > 1.0f)
		{
			m_rate = 1.0f;
		}
	}
}

//-----------------------------------------------------------------
// トランジションマスクを描画
//-----------------------------------------------------------------

void TransitionMask::Draw(
	ID3D11DeviceContext* context,
	DirectX::CommonStates* states, 
	ID3D11ShaderResourceView* texture,
	const RECT& rect)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	context->Map(m_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	static_cast<ConstantBuffer*>(mappedResource.pData)->rate = m_rate;
	context->Unmap(m_constantBuffer.Get(), 0);

	m_spriteBatch->Begin(
		SpriteSortMode_Immediate,
		states->NonPremultiplied(),
		nullptr,
		nullptr,
		nullptr,
		[&]()
		{
			//定数バッファ
			ID3D11Buffer* cbuf = { m_constantBuffer.Get() };

			context->PSSetConstantBuffers(1, 1, &cbuf);

			//マスク用テクスチャ
			context->PSSetShaderResources(1, 1, m_maskTexture.GetAddressOf());

			switch (m_currentFadeType)
			{
			case FadeType::WIPE:
				context->PSSetShader(m_fade_PS.Get(), nullptr, 0);
				break;

				/*case FadeType::CHECKER:
					context->PSSetShader(m_PS_FadeSe.Get(), nullptr, 0);
					break;*/

			}
		}
	);

	//マスクの描画
	//m_spriteBatch->Draw(texture, rect);

	ID3D11ShaderResourceView* useTexture = texture;
	if (useTexture == nullptr)
	{
		useTexture = m_dummyTexture.Get();
	}

	// 色を決める
	DirectX::XMVECTOR color = texture ? DirectX::Colors::White : DirectX::Colors::Black;

	// 決定したテクスチャと色で描画
	m_spriteBatch->Draw(useTexture, rect, color);


	m_spriteBatch->End();

}

//-----------------------------------------------------------------
// 画面の演出を始める　マスクをオープンする関数
//-----------------------------------------------------------------

void TransitionMask::Open()
{
	m_open = true;
	m_rate = 1.0f;
}

//-----------------------------------------------------------------
// 画面の演出を終わる　マスクをクローズする関数
//-----------------------------------------------------------------

void TransitionMask::Close()
{
	m_open = false;
	m_rate = 0.0f;
}
