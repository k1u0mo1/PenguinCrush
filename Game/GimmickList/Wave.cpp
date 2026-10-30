
/**
 * @file   Wave.h
 * @brief  水面の波を動的に生成・描画し、物理的な高さや傾きを提供するギミッククラス
 * @author 國田知睦
 * @date   2026/10/28
 */

#include "pch.h"
#include <fstream>
#include <sstream>

#include "Wave.h"
#include <Game/Common/ReadData.h>

#include "Tool/ImGui/imgui.h"

//----------------------------------------------------------
// 波のコンストラクタ
//----------------------------------------------------------

Wave::Wave(DX::DeviceResources* deviceResources)
    : 
    m_deviceResources(deviceResources)
{
    m_deviceResources->RegisterDeviceNotify(this);
}

//----------------------------------------------------------
// 波の初期化
//----------------------------------------------------------

void Wave::Initialize(HWND hwnd, int width, int height)
{
    UNREFERENCED_PARAMETER(hwnd);

    CreateDeviceResources();
    CreateWindowSizeResources(width, height);

    //波のパラメータの保存データがあれば自動で読み込み
    LoadParameters("wave_config.json");
}

//----------------------------------------------------------
// 波の更新
//----------------------------------------------------------

void Wave::Update(float deltaTime)
{
    //1フレームの時間経過を反映
    m_time += deltaTime;
}

//----------------------------------------------------------
// 波の描画
//----------------------------------------------------------

void Wave::Render(
    ID3D11DeviceContext* context,
    const DirectX::SimpleMath::Matrix& view,
    const DirectX::SimpleMath::Matrix& proj)
{
	//波のスケール　ドットモードなら1.0f、通常モードなら定数で指定したスケール
    /*float scaleX = m_isDotMode ? 1.0f : WAVE_SCALE_X;
    float scaleY = m_isDotMode ? 1.0f : WAVE_SCALE_Y;
    float scaleZ = m_isDotMode ? 1.0f : WAVE_SCALE_Z;*/

    float scaleX = WAVE_SCALE_X;
    float scaleY = WAVE_SCALE_Y;
    float scaleZ = WAVE_SCALE_Z;

    //波のサイズと座標
    DirectX::SimpleMath::Matrix world =
        DirectX::SimpleMath::Matrix::CreateScale(scaleX, scaleY, scaleZ) *
        DirectX::SimpleMath::Matrix::CreateTranslation(0.0f, WAVE_OFFSET_Y, 0.0f);

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

        //シェーダに波の細かさを送る
        cb->waveFrequency = m_uiFrequency; 
        //シェーダに波の高さを送る
        cb->waveAmplitude =  m_uiAmplitude;
        //シェーダに波の速さを送る
        cb->waveSpeed     =  m_uiSpeed;
        //波の尖り具合を送る
        cb->waveSharpness =  m_uiSharpness;

        //シェーダに波の大きさを送る
        cb->cubeScale = m_uiCubeScale;
        //シェーダにグリッドの幅を送る
        cb->gridWidth = 200;

        //色データ
        cb->deepColor = DirectX::SimpleMath::Vector4(
            m_waterColorDeep[0],
            m_waterColorDeep[1],
            m_waterColorDeep[2],
            m_waterColorDeep[3]
        );
        cb->shallowColor = DirectX::SimpleMath::Vector4(
            m_waterColorShallow[0],
            m_waterColorShallow[1],
            m_waterColorShallow[2],
            m_waterColorShallow[3]
        );
		
        //マッピングを解除してGPUに渡す
        context->Unmap(m_constantBuffer.Get(), 0);
    }

    //シェーダをセットして描画
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->VSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

    context->PSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());

    //定数バッファをセット
    context->IASetInputLayout(m_inputLayout.Get());
    //カリング設定（無効）
    context->RSSetState(m_states->CullNone());
    //カメラから近いモノから描画
    context->OMSetDepthStencilState(m_states->DepthDefault(), 0);

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

		//まとめて立方体を描画
        context->DrawIndexedInstanced(
            CUBE_INDEX_COUNT, CUBE_INSTANCE_COUNT, 0, 0, 0);
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
// パラメータ調整
//----------------------------------------------------------

void Wave::RenderUI()
{
#ifdef _DEBUG

    if (!m_showUI)return;

    //GUIウィンドウの開始
    ImGui::Begin("Wave Parameter Inspector");

    ImGui::Text("Mode Select");
    if (ImGui::RadioButton("Mesh Wave", !m_isDotMode))
    {
        m_isDotMode = false;
    }

    ImGui::SameLine();
    if (ImGui::RadioButton("Cube Wave", m_isDotMode))
    {
        m_isDotMode = true;
    }

    ImGui::Separator();
    ImGui::Text("Dot WaveSetting");
    ImGui::SliderFloat("Cube Scale", &m_uiCubeScale, 1.0f, 5.0f);

    ImGui::Text("Physical Parameters");

    //スライダーで数値を動かせるようにする
    ImGui::SliderFloat("Frequency", &m_uiFrequency, 0.05f, 1.0f);
    ImGui::SliderFloat("Amplitude", &m_uiAmplitude, 0.5f,  5.0f);
    ImGui::SliderFloat("Speed",     &m_uiSpeed,     0.1f,  3.0f);
    ImGui::SliderFloat("Sharpness", &m_uiSharpness, 1.0f,  4.0f);
    ImGui::ColorEdit4("Deep Water Color", m_waterColorDeep);
    ImGui::ColorEdit4("Shallow Water Color", m_waterColorShallow);
    

    //パラメータを保存
    if (ImGui::Button("Save Config"))
    {
        SaveParameters("wave_config.json");
    }
    ImGui::SameLine();
    //パラメータを読み込み
    if (ImGui::Button("Load Config"))
    {
        LoadParameters("wave_config.json");
    }
    ImGui::SameLine();

    //パラメータをリセット
    if (ImGui::Button("Reset to Default"))
    {
        m_uiFrequency = 0.25f;
        m_uiAmplitude =  3.5f;
        m_uiSpeed     =  1.2f;
        m_uiSharpness =  2.2f;
    }

    ImGui::End();

#endif
}

//----------------------------------------------------------
// 波のパラメータテキストファイルに保存する
//----------------------------------------------------------

bool Wave::SaveParameters(const std::string& filePath) const
{
    std::ofstream file(filePath);
    if (!file.is_open())
    {
        return false;
    }

    //数値の書き出し
    file << "{\n";
    file << " \"Frequency\":" << m_uiFrequency << ",\n";
    file << " \"Amplitude\":" << m_uiAmplitude << ",\n";
    file << " \"Speed\":"     << m_uiSpeed     << ",\n";
    file << " \"Sharpness\":" << m_uiSharpness << "\n";
    file << "}\n";

    file.close();
    return true;
}

//----------------------------------------------------------
// 波のパラメータテキストファイルから読み込み
//----------------------------------------------------------

bool Wave::LoadParameters(const std::string& filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        //ファイルが存在していなければ初期値
        return false;
    }

    std::string line;
    while (std::getline(file, line))
    {
        //コロンや':'の位置を探す
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos)
        {
            //コロンより後ろの文字の文字列を取り出す
            std::string valueStr = line.substr(colonPos + 1);

            try
            {
                //簡易テキストの解析
                if (line.find("Frequency") != std::string::npos)
                {
                    m_uiFrequency = std::stof(valueStr);
                }
                else if (line.find("Amplitude") != std::string::npos)
                {
                    m_uiAmplitude = std::stof(valueStr);
                }
                else if (line.find("Speed") != std::string::npos)
                {
                    m_uiSpeed = std::stof(valueStr);
                }
                else if (line.find("Sharpness") != std::string::npos)
                {
                    m_uiSharpness = std::stof(valueStr);
                }
            }
            catch(...)
            {

            }
        }
        
    }

    file.close();
    return true;
}

//----------------------------------------------------------
// リソース
//----------------------------------------------------------

void Wave::CreateDeviceResources()
{
    auto device = m_deviceResources->GetD3DDevice();

    device;
    //共通ステートの作成
    m_states = std::make_unique<DirectX::CommonStates>(device);
    
    //波の中心を（０，０，０） 計算
    const float offsetX = (GRID_WIDTH - 1) * GRID_SPACING *0.5f;
    const float offsetZ = (GRID_HEIGHT - 1) * GRID_SPACING * 0.5f;
    
    m_waveVertices.clear();
    //波の頂点データを作成
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
        //{ "SV_InstanceID", 0, DXGI_FORMAT_R32_UINT,  1,  0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
    };
    //入力レイアウトの作成
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
            //頂点のインデックスを計算
            uint16_t i0 = static_cast<uint16_t>(z * GRID_WIDTH + x);
            uint16_t i1 = static_cast<uint16_t>(z * GRID_WIDTH + (x + 1));
            uint16_t i2 = static_cast<uint16_t>((z + 1) * GRID_WIDTH + x);
            uint16_t i3 = static_cast<uint16_t>((z + 1) * GRID_WIDTH + (x + 1));

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
    //頂点データをGPUに渡す
    D3D11_SUBRESOURCE_DATA vinitData = {};
    vinitData.pSysMem = m_waveVertices.data();
    DX::ThrowIfFailed(device->CreateBuffer(&vbd, &vinitData, m_vertexBuffer.GetAddressOf()));

    //インデックスバッファをGPUに作成
    D3D11_BUFFER_DESC ibd = {};
    ibd.Usage = D3D11_USAGE_DEFAULT;
    ibd.ByteWidth = sizeof(uint16_t) * m_indexCount;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    //インデックスデータをGPUに渡す
    D3D11_SUBRESOURCE_DATA iinitData = {};
    iinitData.pSysMem = indices.data();
    DX::ThrowIfFailed(device->CreateBuffer(&ibd, &iinitData, m_indexBuffer.GetAddressOf()));

    //立方体の頂点バッファとインデックスバッファを作成
    CreateCubeBuffer();
}

//----------------------------------------------------------
// 画面リソース
//----------------------------------------------------------

void Wave::CreateWindowSizeResources(int width, int height)
{
    UNREFERENCED_PARAMETER(width);
    UNREFERENCED_PARAMETER(height);

    // 画面サイズの取得
    RECT rect = m_deviceResources->GetOutputSize();

    // 射影行列の作成
    m_proj = DirectX::SimpleMath::Matrix::CreatePerspectiveFieldOfView(
        DirectX::XMConvertToRadians(CAM_FOV)
        , static_cast<float>(rect.right) / static_cast<float>(rect.bottom)
        , CAM_NEAR, CAM_FAR);
}

void Wave::CreateCubeBuffer()
{
    auto device = m_deviceResources->GetD3DDevice();

    //頂点データ
    //立方体の頂点座標と色を定義
    WaveVertex cv[CUBE_SIZE] = {
        { DirectX::SimpleMath::Vector3(-CUDE_SIZE,  CUDE_SIZE, -CUDE_SIZE), DirectX::SimpleMath::Vector4(0.0f, 1.0f, 0.8f, 1.0f) },
        { DirectX::SimpleMath::Vector3( CUDE_SIZE,  CUDE_SIZE, -CUDE_SIZE), DirectX::SimpleMath::Vector4(0.0f, 1.0f, 0.8f, 1.0f) },
        { DirectX::SimpleMath::Vector3( CUDE_SIZE,  CUDE_SIZE,  CUDE_SIZE), DirectX::SimpleMath::Vector4(0.0f, 1.0f, 0.8f, 1.0f) },
        { DirectX::SimpleMath::Vector3(-CUDE_SIZE,  CUDE_SIZE,  CUDE_SIZE), DirectX::SimpleMath::Vector4(0.0f, 1.0f, 0.8f, 1.0f) },
        { DirectX::SimpleMath::Vector3(-CUDE_SIZE, -CUDE_SIZE, -CUDE_SIZE), DirectX::SimpleMath::Vector4(0.0f, 0.5f, 0.8f, 1.0f) },
        { DirectX::SimpleMath::Vector3( CUDE_SIZE, -CUDE_SIZE, -CUDE_SIZE), DirectX::SimpleMath::Vector4(0.0f, 0.5f, 0.8f, 1.0f) },
        { DirectX::SimpleMath::Vector3( CUDE_SIZE, -CUDE_SIZE,  CUDE_SIZE), DirectX::SimpleMath::Vector4(0.0f, 0.5f, 0.8f, 1.0f) },
        { DirectX::SimpleMath::Vector3(-CUDE_SIZE, -CUDE_SIZE,  CUDE_SIZE), DirectX::SimpleMath::Vector4(0.0f, 0.5f, 0.8f, 1.0f) },
    };

    //頂点をつなげる
    const uint16_t ci[CUBE_INDEX_COUNT] = {
        0,1,2, 0,2,3,//上
        4,6,5 ,4,7,6,//下
        4,5,1, 4,1,0,//手前
        7,3,2, 7,2,6,//奥
        4,0,3, 4,3,7,//左
        5,6,2, 5,2,1 //右
    };

    //頂点バッファの作成
    D3D11_BUFFER_DESC CubeVertexBufferDesc = {};
    CubeVertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    CubeVertexBufferDesc.ByteWidth = sizeof(WaveVertex) * CUBE_SIZE;
    CubeVertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA cubeVertexSubresourceData = {};
    cubeVertexSubresourceData.pSysMem = cv;
    DX::ThrowIfFailed(device->CreateBuffer(&CubeVertexBufferDesc, &cubeVertexSubresourceData, m_cubeVertexBuffer.ReleaseAndGetAddressOf()));

    //インデックスバッファの作成
    D3D11_BUFFER_DESC CubeIndexBufferDesc = {};
    CubeIndexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    CubeIndexBufferDesc.ByteWidth = sizeof(uint16_t) * CUBE_INDEX_COUNT;
    CubeIndexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    D3D11_SUBRESOURCE_DATA cubeIndexSubresourceData = {};
    cubeIndexSubresourceData.pSysMem = ci;
    DX::ThrowIfFailed(device->CreateBuffer(&CubeIndexBufferDesc, &cubeIndexSubresourceData, m_cubeIndexBuffer.ReleaseAndGetAddressOf()));
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
    //座標をずらして実際の高さを調べて傾きを計算
    const float epsilon = GRADIENT_CALC_OFFSET;

    //Ｘ方向の傾き(少し右の高さ - 少し左の高さ)
    float h_minus_x = GetHeight(x - epsilon, z);
    float h_plus_x = GetHeight(x + epsilon, z);
    float dy_dx = (h_plus_x - h_minus_x) / (GRADIENT_CALC_DENOMINATOR);

    //Ｚ方向の傾き(少し奥の高さ - 少し手前の高さ)
    float h_minus_z = GetHeight(x, z - epsilon);
    float h_plus_z = GetHeight(x, z + epsilon);
    float dy_dz = (h_plus_z - h_minus_z) / (GRADIENT_CALC_DENOMINATOR);

    return { dy_dz,dy_dx };
}

//----------------------------------------------------------
// 指定された座標 (X, Z) における波の現在高さを取得
//----------------------------------------------------------
//波の高さを取得

float Wave::GetHeight(float x, float z) const
{
    //ワールド座標(x, z)を、波の計算に使うグリッド座標(fx, fz)に逆変換
    float offsetX = (GRID_WIDTH - 1) * GRID_SPACING * HALF_COEFFICIENT;
    float offsetZ = (GRID_HEIGHT - 1) * GRID_SPACING * HALF_COEFFICIENT;

    //WAVE_SCALE で割りオフセットを合わせて間隔(SPACING)で割る
    float fx = (x / WAVE_SCALE_X + offsetX) / GRID_SPACING;
    float fz = (z / WAVE_SCALE_Z + offsetZ) / GRID_SPACING;

    //グリッド座標で高さを計算
    float y = CalculateHeight(fx, fz, m_time);

    //Renderで適用しているワールド行列と同じスケールとオフセットをY軸に適用
    return y * WAVE_SCALE_Y + WAVE_OFFSET_Y;
}