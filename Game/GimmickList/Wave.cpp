//
// Wave.cpp
//

#include "pch.h"
#include "Wave.h"
#include <Game/Common/ReadData.h>



using namespace DirectX;

using Microsoft::WRL::ComPtr;

//----------------------------------------------------------
// 波のコンストラクタ
//----------------------------------------------------------

Wave::Wave(DX::DeviceResources* deviceResources)
    : m_deviceResources(deviceResources)

{
    m_deviceResources->RegisterDeviceNotify(this);
}

//----------------------------------------------------------
// 波の初期化
//----------------------------------------------------------

void Wave::Initialize(HWND /*hwnd*/, int width, int height)
{
    CreateDeviceResources();
    CreateWindowSizeResources(width, height);
}

//----------------------------------------------------------
// 波の更新
//----------------------------------------------------------

void Wave::Update(float deltaTime)
{
    //1フレームの時間経過を反映
    m_time += deltaTime;

    //波の更新
    //UpdateWaveVertices();

}

//----------------------------------------------------------
// 波の描画
//----------------------------------------------------------

void Wave::Render(ID3D11DeviceContext* context, const SimpleMath::Matrix& view, const SimpleMath::Matrix& proj)
{
	//波のスケール　ドットモードなら1.0f、通常モードなら定数で指定したスケール
    float scaleX = m_isDotMode ? 1.0f : WAVE_SCALE_X;
    float scaleY = m_isDotMode ? 1.0f : WAVE_SCALE_Y;
    float scaleZ = m_isDotMode ? 1.0f : WAVE_SCALE_Z;

    //波のサイズと座標
    SimpleMath::Matrix world =
        SimpleMath::Matrix::CreateScale(scaleX, scaleY, scaleZ) *
        SimpleMath::Matrix::CreateTranslation(0.0f, WAVE_OFFSET_Y, 0.0f);

    //GPUに時間と行列を送る
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    if (SUCCEEDED(context->Map(m_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
    {
        auto cb = (WaveConstantBuffer*)mappedResource.pData;
        cb->matWorld = world.Transpose();
        cb->matView = view.Transpose();
        cb->matProj = proj.Transpose();
        cb->time    = m_time;

		//ドットモードなら1、通常モードなら0をシェーダに送る
        cb->isCubeMode = m_isDotMode ? 1 : 0;
		//構造体のサイズを16バイトの倍数にするためのパディング
        cb->padding = DirectX::SimpleMath::Vector2(0.0f, 0.0f);

        context->Unmap(m_constantBuffer.Get(), 0);
    }

    //シェーダをセットして描画
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->VSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

    context->IASetInputLayout(m_inputLayout.Get());
    //カリング設定（無効）
    context->RSSetState(m_states->CullNone());

    //頂点バッファをセット
    UINT stride = sizeof(WaveVertex);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);

    
    //ドットの波モード
    if (m_isDotMode)
    {
		//頂点バッファとインデックスバッファをセット
        context->IASetVertexBuffers(0, 1, m_cubeVertexBuffer.GetAddressOf(), &stride, &offset);
		//インデックスバッファを使って面を描画
        context->IASetIndexBuffer(m_cubeIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
		//三角形の面として描画設定
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//まとめて描画　10000個の立方体を描画
        context->DrawIndexedInstanced(36, 40000, 0, 0, 0);
    }
    //通常の波モード
    else
    {
		//頂点バッファとインデックスバッファをセット
        context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
		//インデックスバッファを使って面を描画
        context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
		//三角形の面として描画設定
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        //まとめて描画
        context->DrawIndexed(m_indexCount, 0, 0);
    }
}

//----------------------------------------------------------
// リソース
//----------------------------------------------------------

void Wave::CreateDeviceResources()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto context = m_deviceResources->GetD3DDeviceContext();

    device;

    m_states = std::make_unique<CommonStates>(device);

    
    //波の中心を（０，０，０） 計算
    const float offsetX = (GRID_WIDTH - 1) * GRID_SPACING *0.5f;
    const float offsetZ = (GRID_HEIGHT - 1) * GRID_SPACING * 0.5f;

    m_waveVertices.clear();
    for (int z = 0; z < GRID_HEIGHT; z++)
    {
        for (int x = 0; x < GRID_WIDTH; x++)
        {
            WaveVertex v;

            v.position =
            {
                x * GRID_SPACING - offsetX,
                0.0f,
                z * GRID_SPACING - offsetZ
            };
            v.color = DirectX::Colors::Blue;

            m_waveVertices.push_back(v);
        }
    }

    //頂点シェーダーの読み込みと作成
    auto vertexShaderBlob = DX::ReadData(L"Resources\\Shaders\\WaveVS.cso");
    DX::ThrowIfFailed(device->CreateVertexShader(
        vertexShaderBlob.data(), vertexShaderBlob.size(),
        nullptr, m_vertexShader.GetAddressOf()
    ));

    //入力レイアウトの作成
    static const D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
    {
        {"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0, 0, D3D11_INPUT_PER_VERTEX_DATA,0},
        {"COLOR",   0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,12, D3D11_INPUT_PER_VERTEX_DATA,0},
    };

    DX::ThrowIfFailed(device->CreateInputLayout(
        inputElementDesc, ARRAYSIZE(inputElementDesc),
        vertexShaderBlob.data(), vertexShaderBlob.size(),
        m_inputLayout.GetAddressOf()
    ));

    //ピクセルシェーダの読み込み
    auto pixelShaderBlob = DX::ReadData(L"Resources\\Shaders\\WavePS.cso");
    DX::ThrowIfFailed(device->CreatePixelShader(
        pixelShaderBlob.data(), pixelShaderBlob.size(),
        nullptr, m_pixelShader.GetAddressOf()
    ));

    //定数バッファの作成
    D3D11_BUFFER_DESC cbd = {};
    cbd.Usage = D3D11_USAGE_DYNAMIC;
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    cbd.ByteWidth = sizeof(WaveConstantBuffer);
    DX::ThrowIfFailed(device->CreateBuffer(&cbd, nullptr, m_constantBuffer.GetAddressOf()));

    //インデックスデータを作成 頂点の結び順
    std::vector<uint16_t> indices;
    for (int z = 0; z < GRID_HEIGHT - 1; z++)
    {
        for (int x = 0; x < GRID_WIDTH - 1; x++)
        {
            int i0 = z * GRID_WIDTH + x;
            int i1 = z * GRID_WIDTH + (x + 1);
            int i2 = (z + 1) * GRID_WIDTH + x;
            int i3 = (z + 1) * GRID_WIDTH + (x + 1);

            //１つ目の三角形
            indices.push_back(i0);
            indices.push_back(i1);
            indices.push_back(i2);

            //２つ目の三角形
            indices.push_back(i2);
            indices.push_back(i3);
            indices.push_back(i1);
        }
    }
    m_indexCount = static_cast<UINT>(indices.size());

    //頂点バッファをGPUに作成
    D3D11_BUFFER_DESC vbd = {};
    vbd.Usage = D3D11_USAGE_DEFAULT;
    vbd.ByteWidth = sizeof(WaveVertex) * static_cast<UINT>(m_waveVertices.size());
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vinitData = {};
    vinitData.pSysMem = m_waveVertices.data();
    DX::ThrowIfFailed(device->CreateBuffer(&vbd, &vinitData, m_vertexBuffer.GetAddressOf()));

    //インデックスバッファをGPUに作成
    D3D11_BUFFER_DESC ibd = {};
    ibd.Usage = D3D11_USAGE_DEFAULT;
    ibd.ByteWidth = sizeof(uint16_t) * m_indexCount;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA iinitData = {};
    iinitData.pSysMem = indices.data();
    DX::ThrowIfFailed(device->CreateBuffer(&ibd, &iinitData, m_indexBuffer.GetAddressOf()));

    //
    CreateCubeBuffer();

}

//----------------------------------------------------------
// 画面リソース
//----------------------------------------------------------

void Wave::CreateWindowSizeResources(int /*width*/, int /*height*/)
{
    // // 画面サイズの取得
    RECT rect = m_deviceResources->GetOutputSize();

    // 射影行列の作成
    m_proj = SimpleMath::Matrix::CreatePerspectiveFieldOfView(
        XMConvertToRadians(45.0f)
        , static_cast<float>(rect.right) / static_cast<float>(rect.bottom)
        , 0.1f, 1000.0f);
}

void Wave::CreateCubeBuffer()
{
    auto device = m_deviceResources->GetD3DDevice();

    //頂点データ
    WaveVertex cv[8] = {
        { DirectX::SimpleMath::Vector3(-0.2f,  0.2f, -0.2f), DirectX::SimpleMath::Vector4(0.0f, 1.0f, 0.8f, 1.0f) },
        { DirectX::SimpleMath::Vector3( 0.2f,  0.2f, -0.2f), DirectX::SimpleMath::Vector4(0.0f, 1.0f, 0.8f, 1.0f) },
        { DirectX::SimpleMath::Vector3( 0.2f,  0.2f,  0.2f), DirectX::SimpleMath::Vector4(0.0f, 1.0f, 0.8f, 1.0f) },
        { DirectX::SimpleMath::Vector3(-0.2f,  0.2f,  0.2f), DirectX::SimpleMath::Vector4(0.0f, 1.0f, 0.8f, 1.0f) },
        { DirectX::SimpleMath::Vector3(-0.2f, -0.2f, -0.2f), DirectX::SimpleMath::Vector4(0.0f, 0.5f, 0.8f, 1.0f) },
        { DirectX::SimpleMath::Vector3( 0.2f, -0.2f, -0.2f), DirectX::SimpleMath::Vector4(0.0f, 0.5f, 0.8f, 1.0f) },
        { DirectX::SimpleMath::Vector3( 0.2f, -0.2f,  0.2f), DirectX::SimpleMath::Vector4(0.0f, 0.5f, 0.8f, 1.0f) },
        { DirectX::SimpleMath::Vector3(-0.2f, -0.2f,  0.2f), DirectX::SimpleMath::Vector4(0.0f, 0.5f, 0.8f, 1.0f) },
    };

    //頂点をつなげる
    const uint16_t ci[36] = {
        0,1,2, 0,2,3,//上
        4,6,5 ,4,7,6,//下
        4,5,1, 4,1,0,//手前
        7,3,2, 7,2,6,//奥
        4,0,3, 4,3,7,//左
        5,6,2, 5,2,1 //右
    };

    //頂点バッファの作成
    D3D11_BUFFER_DESC cvbd = {};
    cvbd.Usage = D3D11_USAGE_DEFAULT;
    cvbd.ByteWidth = sizeof(WaveVertex) * 8; //8つの頂点
    cvbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA cvsd = {};
    cvsd.pSysMem = cv;
    DX::ThrowIfFailed(device->CreateBuffer(&cvbd, &cvsd, m_cubeVertexBuffer.ReleaseAndGetAddressOf()));

    //インデックスバッファの作成
    D3D11_BUFFER_DESC cibd = {};
    cibd.Usage = D3D11_USAGE_DEFAULT;
    cibd.ByteWidth = sizeof(uint16_t) * 36; // 36インデックス
    cibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    D3D11_SUBRESOURCE_DATA cisd = {};
    cisd.pSysMem = ci;
    DX::ThrowIfFailed(device->CreateBuffer(&cibd, &cisd, m_cubeIndexBuffer.ReleaseAndGetAddressOf()));
}

//----------------------------------------------------------
// 波の更新
//----------------------------------------------------------
//毎フレーム、頂点をYをsinとcosで揺らす
//波を座標０を中心にするためのやつ 高さ計算修正

void Wave::UpdateWaveVertices()
{
 
    const float offsetX = (GRID_WIDTH - 1) * GRID_SPACING * 0.5f;
    const float offsetZ = (GRID_WIDTH - 1) * GRID_SPACING * 0.5f;

    for (int z = 0; z < GRID_HEIGHT; z++)
    {
        for (int x = 0; x < GRID_WIDTH; x++)
        {
            size_t index = static_cast<size_t>(z * GRID_WIDTH + x);

            auto& vertex = m_waveVertices[index];

            float fx = static_cast<float>(x);
            float fz = static_cast<float>(z);

            //ヘルパー関数で計算したものを使用
            float y = CalculateHeight(fx, fz, m_time);

            vertex.position = DirectX::SimpleMath::Vector3(
                fx * GRID_SPACING - offsetX,
                y,
                fz * GRID_SPACING - offsetZ
            );

            //ヘルパー関数で色を決める
            vertex.color = CalculateColor(y);
        }
    }

}

void Wave::OnDeviceLost() {}
void Wave::OnDeviceRestored() {}

//----------------------------------------------------------
// 波オブジェクトの基準座標を取得
//----------------------------------------------------------

DirectX::SimpleMath::Vector3 Wave::GetPosition() const
{
    return m_position;
}

//----------------------------------------------------------
// 指定された座標 (X, Z) における波の揺れ角度（勾配）を取得
//----------------------------------------------------------
//波の揺れ角度をステージと共有

DirectX::SimpleMath::Vector2 Wave::GetWaveAngle(float x, float z) const
{
   
    //float fx = x;
    //float fz = z;

    //// 勾配の計算（WAVE_FREQUENCY を使用）
    //float dy_dx = cosf(fx * WAVE_FREQUENCY + m_time) * WAVE_FREQUENCY;
    //float dy_dz = -sinf(fz * WAVE_FREQUENCY + m_time) * WAVE_FREQUENCY;

    //return { dy_dz, dy_dx };

    //○.○fだけ座標をずらして実際の高さを調べて傾きを計算
    const float epsilon = 0.1f;

    //Ｘ方向の傾き(少し右の高さ - 少し左の高さ)
    float h_minus_x = GetHeight(x - epsilon, z);
    float h_plus_x = GetHeight(x + epsilon, z);
    float dy_dx = (h_plus_x - h_minus_x) / (epsilon * 2.0f);

    //Ｚ方向の傾き(少し奥の高さ - 少し手前の高さ)
    float h_minus_z = GetHeight(x, z - epsilon);
    float h_plus_z = GetHeight(x, z + epsilon);
    float dy_dz = (h_plus_z - h_minus_z) / (epsilon * 2.0f);

    return { dy_dz,dy_dx };
   
}

//----------------------------------------------------------
// 指定された座標 (X, Z) における波の現在高さを取得
//----------------------------------------------------------
//波の高さを取得

float Wave::GetHeight(float x, float z) const
{
    //ワールド座標(x, z)を、波の計算に使うグリッド座標(fx, fz)に逆変換
    float offsetX = (GRID_WIDTH - 1) * GRID_SPACING * 0.5f;
    float offsetZ = (GRID_HEIGHT - 1) * GRID_SPACING * 0.5f;

    //WAVE_SCALE で割りオフセットを合わせて間隔(SPACING)で割る
    float fx = (x / WAVE_SCALE_X + offsetX) / GRID_SPACING;
    float fz = (z / WAVE_SCALE_Z + offsetZ) / GRID_SPACING;

    //グリッド座標で高さを計算
    float y = CalculateHeight(fx, fz, m_time);

    //Renderで適用しているワールド行列と同じスケールとオフセットをY軸に適用
    return y * WAVE_SCALE_Y + WAVE_OFFSET_Y;

    //return CalculateHeight(x, z, m_time, 1.0f);
}



