#include "pch.h"
#include "Rain.h"

#include <Library/BinaryFile.h>

//----------------------------------------------------------
// 初期化（オーバーライド）
//----------------------------------------------------------

void Rain::Initialize(ID3D11Device* device)
{
	//親クラスの初期化
	WeatherBase::Initialize(device);

	//シェーダファイルを読み込む
    BinaryFile VSData = BinaryFile::LoadFile(L"Resources/Shaders/RainVS.cso");
    BinaryFile GSData = BinaryFile::LoadFile(L"Resources/Shaders/RainGS.cso");
    BinaryFile PSData = BinaryFile::LoadFile(L"Resources/Shaders/RainPS.cso");

    device->CreateVertexShader(VSData.GetData(), VSData.GetSize(), nullptr, m_vs.ReleaseAndGetAddressOf());

    // ジオメトリシェーダ作成
    device->CreateGeometryShader(GSData.GetData(), GSData.GetSize(), nullptr, m_gs.ReleaseAndGetAddressOf());

    // ピクセルシェーダ作成
    device->CreatePixelShader(PSData.GetData(), PSData.GetSize(), nullptr, m_ps.ReleaseAndGetAddressOf());

    // 入力レイアウト作成
    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    // VSDataを使ってレイアウトを作成
    device->CreateInputLayout(layout, 2, VSData.GetData(), VSData.GetSize(), m_layout.ReleaseAndGetAddressOf());
}

//----------------------------------------------------------
// 描画（オーバーライド）
//----------------------------------------------------------

void Rain::Render(ID3D11DeviceContext* context, const DirectX::SimpleMath::Matrix& view, const DirectX::SimpleMath::Matrix& proj, const DirectX::SimpleMath::Vector3& camPos)
{
    //定数バッファの更新
    CBWeather cb;
    cb.view = view.Transpose();
    cb.proj = proj.Transpose();

    //時間計測 
    cb.time = DirectX::SimpleMath::Vector4(m_time, 0, 0, 0); // 雨なのでTypeフラグは不要(0でOK)

    cb.cameraPos = DirectX::SimpleMath::Vector4(camPos.x, camPos.y, camPos.z, 1);

    //雨パラメータ 
    cb.params = DirectX::SimpleMath::Vector4(20.0f, 0.1f, 0, 0);

    context->UpdateSubresource(m_constBuffer.Get(), 0, nullptr, &cb, 0, 0);

    //シェーダとステートのセット
    //親クラスで作った共通設定
    UINT stride = sizeof(DirectX::SimpleMath::Vector3) + sizeof(DirectX::SimpleMath::Vector2);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

    context->VSSetConstantBuffers(0, 1, m_constBuffer.GetAddressOf());
    context->GSSetConstantBuffers(0, 1, m_constBuffer.GetAddressOf());

    // 雨専用の設定
    context->IASetInputLayout(m_layout.Get());
    context->VSSetShader(m_vs.Get(), nullptr, 0);
    context->GSSetShader(m_gs.Get(), nullptr, 0);
    context->PSSetShader(m_ps.Get(), nullptr, 0);

    // ブレンドステート
    context->OMSetBlendState(m_states->NonPremultiplied(), nullptr, 0xFFFFFFFF);
    context->OMSetDepthStencilState(m_states->DepthRead(), 0);
    context->RSSetState(m_states->CullNone());

    // 描画
    context->Draw(1000, 0);

    // 後片付け
    context->GSSetShader(nullptr, nullptr, 0);
}
