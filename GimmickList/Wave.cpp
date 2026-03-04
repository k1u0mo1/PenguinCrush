//
// Wave.cpp
//

#include "pch.h"
#include "Wave.h"

//ランダム用
#include <random>

using namespace DirectX;

using Microsoft::WRL::ComPtr;

//コンストラクタ
Wave::Wave(DX::DeviceResources* deviceResources)
    : m_deviceResources(deviceResources)

{
    m_deviceResources->RegisterDeviceNotify(this);
}

//初期化
void Wave::Initialize(HWND /*hwnd*/, int width, int height)
{
    CreateDeviceResources();
    CreateWindowSizeResources(width, height);
}

void Wave::Update(float elapsedTime)
{
    //時間経過を反映
    m_time += elapsedTime;

    //波の更新
    UpdateWaveVertices();

}

void Wave::Render(ID3D11DeviceContext* context, const SimpleMath::Matrix& view, const SimpleMath::Matrix& proj)
{
    //波のサイズと座標
    SimpleMath::Matrix world =
        SimpleMath::Matrix::CreateScale(10.0f, 1.0f, 10.0f) *
        SimpleMath::Matrix::CreateTranslation(0.0f, -2.0f, 0.0f);
        //SimpleMath::Matrix::CreateTranslation(0.0f, -4.0f, 0.0f);


    //波の描画処理
    m_effect->SetWorld(world);

    m_effect->SetView(view);

    m_effect->SetProjection(proj);


    m_effect->Apply(context);


    context->IASetInputLayout(m_inputLayout.Get());

    //カリング設定（無効）
    context->RSSetState(m_states->CullNone());

    m_batch->Begin();

    //大きさ
    const int width = 100;
    const int height = 100;


    //波の作成
    for (int z = 0; z < height - 1; z++)
    {
        for (int x = 0; x < width - 1; x++)
        {
            int i0 = z * width + x;
            int i1 = z * width + (x + 1);
            int i2 = (z + 1) * width + x;
            int i3 = (z + 1) * width + (x + 1);

            m_batch->DrawTriangle(
                { m_waveVertices[i0].position,m_waveVertices[i0].color },
                { m_waveVertices[i1].position,m_waveVertices[i1].color },
                { m_waveVertices[i2].position,m_waveVertices[i2].color }
            );

            m_batch->DrawTriangle(
                { m_waveVertices[i2].position,m_waveVertices[i2].color },
                { m_waveVertices[i3].position,m_waveVertices[i3].color },
                { m_waveVertices[i1].position,m_waveVertices[i1].color }
            );
        }
    }

    m_batch->End();

}

void Wave::CreateDeviceResources()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto context = m_deviceResources->GetD3DDeviceContext();

    device;

    m_states = std::make_unique<CommonStates>(device);

    m_effectFactory = std::make_unique<DirectX::EffectFactory>(device);
    m_effectFactory->SetDirectory(L"Resources\\Models");

    m_batch = std::make_unique<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>>(context);

    m_effect = std::make_unique<BasicEffect>(device);

    m_effect->SetVertexColorEnabled(true);


    void const* shaderByteCode;
    size_t byteCodeLength;
    m_effect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);


    device->CreateInputLayout(VertexPositionColor::InputElements,
        VertexPositionColor::InputElementCount,
        shaderByteCode, byteCodeLength,
        m_inputLayout.GetAddressOf());


    ////////////////////////////////////////////////////////

    //ランダム用
    std::random_device rd;
    std::mt19937 gen(rd());

    //最小値～最大値の間の数値を取り出す　乱数
    std::uniform_real_distribution<float> dist(0.5f, 2.5f);

    ////////////////////////////////////////////////////////

    //面状のグリッドの生成処理

    const int width = 100;
    const int height = 100;
    const float spacing = 0.2f;

    //波の中心を（０，０，０） 計算
    const float offsetX = (width - 1) * spacing * 0.5f;
    const float offsetZ = (height - 1) * spacing * 0.5f;

    m_waveVertices.clear();
    for (int z = 0; z < height; z++)
    {
        for (int x = 0; x < width; x++)
        {
            WaveVertex v;

            v.position =
            {
                x * spacing - offsetX,
                0.0f,
                z * spacing - offsetZ
            };
            v.color = DirectX::Colors::Blue;

            //ランダムな振り幅を設定
            v.amplitude = dist(gen);

            m_waveVertices.push_back(v);
        }
    }

}

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

//毎フレーム、頂点をYをsinとcosで揺らす
//波を座標０を中心にするためのやつ 高さ計算修正
void Wave::UpdateWaveVertices()
{
    //const int width = 100;
    //const int height = 100;

    ////波の細かさ
    //float frequency = 0.4f;

    ////波の動く速さ
    //float speed = 0.05f;

    ////波の高さの最大値 (振幅)
    ////float amplitude = 0.3f;

    ////時間更新
    //m_time += speed * 0.012f;

    //float spacing = 0.2f;
    //float offsetX = (width - 1) * spacing * 0.5f;
    //float offsetZ = (height - 1) * spacing * 0.5f;

    //for (int z = 0; z < height; z++)
    //{
    //    for (int x = 0; x < width; x++)
    //    {
    //        float fx = static_cast<float>(x);
    //        float fz = static_cast<float>(z);

    //        float wave =
    //            sinf(fx * frequency + m_time) +
    //            cosf(fz * frequency + m_time);

    //        //中心が０になるようにする
    //        float y = wave * 0.5f * m_waveVertices[static_cast<std::vector<Wave::WaveVertex, std::allocator<Wave::WaveVertex>>::size_type>(z) * width + x].amplitude;

    //        m_waveVertices[static_cast<std::vector<Wave::WaveVertex, std::allocator<Wave::WaveVertex>>::size_type>(z) * width + x].position =
    //            SimpleMath::Vector3(
    //                fx * spacing - offsetX,//X間隔の調整
    //                y,//波の高さ
    //                fz * spacing - offsetZ//Z間隔の調整
    //            );
    //        m_waveVertices[static_cast<std::vector<Wave::WaveVertex, std::allocator<Wave::WaveVertex>>::size_type>(z) * width + x].color = Colors::Blue;
    //    }
    //}

    //m_time += WAVE_SPEED;

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
            float y = CalculateHeight(fx, fz, m_time, vertex.amplitude);

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

DirectX::SimpleMath::Vector3 Wave::GetPosition() const
{
    return m_position;
}

//波の揺れ角度をステージと共有
DirectX::SimpleMath::Vector2 Wave::GetWaveAngle(float x, float z) const
{
    //float frequency = 0.2f;
    //float fx = x;
    //float fz = z;

    ////勾配
    //float dy_dx = cosf(fx * frequency + m_time) * frequency;
    //float dy_dz = -sinf(fz * frequency * m_time) * frequency;

    //////角度に変える
    ////float angleX = atan(dy_dz); // X軸回転（前後）
    ////float angleZ = -atan(dy_dx);// Z軸回転（左右）

    ////atanしないで傾きのベクトルを渡す
    //return { dy_dz,dy_dx };


    float fx = x;
    float fz = z;

    // 勾配の計算（WAVE_FREQUENCY を使用）
    float dy_dx = cosf(fx * WAVE_FREQUENCY + m_time) * WAVE_FREQUENCY;
    float dy_dz = -sinf(fz * WAVE_FREQUENCY + m_time) * WAVE_FREQUENCY;

    return { dy_dz, dy_dx };

}
//波の高さを取得
float Wave::GetHeight(float x, float z) const
{
    //float frequency = 0.4f;

    //float wave =
    //    sinf(x * frequency + m_time) +
    //    cosf(z * frequency * m_time);

    //// 中心0、高さ調整
    //float y = wave * 0.3f;

    //return y;

    return CalculateHeight(x, z, m_time, 1.0f);
}



