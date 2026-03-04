

#include "pch.h"
#include "Snow.h"

#include "Library/BinaryFile.h"
#include <WICTextureLoader.h>

using namespace DirectX;

void Snow::Initialize(ID3D11Device* device)
{
    // 親クラス初期化
    WeatherBase::Initialize(device);

    // 雪専用シェーダ読み込み 
    BinaryFile VSData = BinaryFile::LoadFile(L"Resources/Shaders/SnowVS.cso");
    BinaryFile GSData = BinaryFile::LoadFile(L"Resources/Shaders/SnowGS.cso");
    BinaryFile PSData = BinaryFile::LoadFile(L"Resources/Shaders/SnowPS.cso");

    device->CreateVertexShader(VSData.GetData(), VSData.GetSize(), nullptr, m_vs.ReleaseAndGetAddressOf());
    device->CreateGeometryShader(GSData.GetData(), GSData.GetSize(), nullptr, m_gs.ReleaseAndGetAddressOf());
    device->CreatePixelShader(PSData.GetData(), PSData.GetSize(), nullptr, m_ps.ReleaseAndGetAddressOf());

    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    device->CreateInputLayout(layout, 2, VSData.GetData(), VSData.GetSize(), m_layout.ReleaseAndGetAddressOf());
    
    //テクスチャ用
    CreateWICTextureFromFile(device, L"Resources/Textures/White.png", nullptr, m_texture.ReleaseAndGetAddressOf());
}

void Snow::Render(
    ID3D11DeviceContext* context,
    const DirectX::SimpleMath::Matrix& view,
    const DirectX::SimpleMath::Matrix& proj,
    const DirectX::SimpleMath::Vector3& camPos)
{
    // 定数バッファ更新
    CBWeather cb;
    cb.view = view.Transpose();
    cb.proj = proj.Transpose();

    // 雪は Type = 2.0f 
    cb.time = DirectX::SimpleMath::Vector4(m_time, 2.0f, 0, 0);

    cb.cameraPos = DirectX::SimpleMath::Vector4(camPos.x, camPos.y, camPos.z, 1);

    // 雪パラメータ
    //cb.params = DirectX::SimpleMath::Vector4(2.0f, 0.5f, camPos.x, camPos.z);
    //カメラに追従しないように
    cb.params = DirectX::SimpleMath::Vector4(1.5f, 0.5f, 0.0f, 0.0f);

    context->UpdateSubresource(m_constBuffer.Get(), 0, nullptr, &cb, 0, 0);

    
    UINT stride = sizeof(DirectX::SimpleMath::Vector3) + sizeof(DirectX::SimpleMath::Vector2);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

    context->VSSetConstantBuffers(0, 1, m_constBuffer.GetAddressOf());
    context->GSSetConstantBuffers(0, 1, m_constBuffer.GetAddressOf());

    // 雪のシェーダをセット
    context->IASetInputLayout(m_layout.Get());
    context->VSSetShader(m_vs.Get(), nullptr, 0);
    context->GSSetShader(m_gs.Get(), nullptr, 0);
    context->PSSetShader(m_ps.Get(), nullptr, 0);

    // テクスチャとサンプラーをセット 
    context->PSSetShaderResources(0, 1, m_texture.GetAddressOf()); // t0 レジスタにセット

    // 画像を綺麗に表示するためのサンプラー
    auto sampler = m_states->LinearClamp();
    context->PSSetSamplers(0, 1, &sampler);

    // ブレンド設定など
    context->OMSetBlendState(m_states->NonPremultiplied(), nullptr, 0xFFFFFFFF);
    context->OMSetDepthStencilState(m_states->DepthRead(), 0);
    context->RSSetState(m_states->CullNone());

    context->Draw(5000, 0);

    context->GSSetShader(nullptr, nullptr, 0);
}