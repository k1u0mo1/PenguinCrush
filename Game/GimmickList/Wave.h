// Wave.h
#pragma once

#include "pch.h"

#include "Game/Common/DeviceResources.h"
#include "Game/Common/StepTimer.h"
#include "Effects.h"

//継承/////////////////////////
#include "Game/GimmickList/IGimmick.h"
///////////////////////////////

#include <vector>
#include <algorithm>
#include <cmath>

//Wave（波）のクラス   [IGimmickに継承]

/// <summary>
/// GPUに送るデータ構造
/// </summary>
struct WaveConstantBuffer
{
    DirectX::SimpleMath::Matrix matWorld;

    DirectX::SimpleMath::Matrix matView;

    DirectX::SimpleMath::Matrix matProj;

    float time;

    int isCubeMode;

    DirectX::SimpleMath::Vector2 padding;
};


/// <summary>
/// 水面の波を動的に生成・描画し、物理的な高さや傾きを提供するギミッククラス
/// </summary>
class Wave : public IGimmick, public DX::IDeviceNotify
{
private:

    //------------------------------------------------------
    //波の描画スケール＆位置関連
    //------------------------------------------------------

    //波のX軸スケール
    static constexpr float WAVE_SCALE_X = 10.0f;
    //波のY軸スケール
    static constexpr float WAVE_SCALE_Y = 1.0f;
    //波のZ軸スケール
    static constexpr float WAVE_SCALE_Z = 10.0f;
    //波の描画位置 （Y軸高さ）
    static constexpr float WAVE_OFFSET_Y = -1.0f;

private:

    /// <summary>
    /// 波を構成する頂点構造体
    /// </summary>
    struct  WaveVertex
    {
        //座標
        DirectX::SimpleMath::Vector3 position;
        //色
        DirectX::SimpleMath::Vector4 color;
        
    };

    //形状と解像度に関する定数-------------
    //高さ
    static constexpr int GRID_WIDTH = 100;
    static constexpr int GRID_HEIGHT = 100;
    //間隔
    static constexpr float GRID_SPACING = 0.5f;

    //波の物理挙動に関する定数-------------
    //波の細かさ
    static constexpr float WAVE_FREQUENCY = 0.15f;
    //時間経過の速さ
    static constexpr float WAVE_SPEED = 0.02f;
    //波の高さ 
    static constexpr float WAVE_AMPLIYUDE = 1.5f;
    //波を配置する高さ
    static constexpr float WORLD_Y = -2.0f;
    //波のスケール
    static constexpr float WORLD_SCALE = 10.0f;

    
    /// <summary>
    /// 波の高さ Y を計算 
    /// UpdateWaveVerticesとGetHeightで使用 ゲームのギミック
    /// </summary>
    /// <param name="x">座標X</param>
    /// <param name="z">座標Y</param>
    /// <param name="time">時間</param>
    /// <returns>計算されたY座標</returns>
    float CalculateHeight(
        float x, float z,
        float time
        )const 
    {

        //頂点のインデックスをワールド座標のスケールに合わせる
        float localX = x * GRID_SPACING;
        float localZ = z * GRID_SPACING;

        //-------------------------------------------
        //複数の波の計算を合成してリアルな波にする
        //-------------------------------------------

        //波１　ベースとなる揺れが大きくゆったり
        float waveSlowly = sinf(localX * WAVE_FREQUENCY + time * 1.0f);

        //波２　少し細かく動きが速い
        float waveQuick = cosf(localZ * WAVE_AMPLIYUDE * 1.5f + time * 1.2f) * 0.5f;

        //波３　斜めの方向へ細かい波
        float waveSlanting = sinf((localX + localZ) * 1.0f + time * 1.0f) * 0.25f;

        //波４　逆斜めの方向の細かい波
        float waveReverse = cosf((localX - localZ)   * 0.25f + time * 0.5f) * 0.5f;

        //すべての波を合成
        float wave = waveSlowly + waveQuick + (waveSlanting - waveReverse);

        //波を返す
        return wave * WAVE_AMPLIYUDE ;
        
    }

    /// <summary>
    /// 高さから色を計算
    /// </summary>
    /// <param name="y">現在の波の高さ</param>
    /// <returns>高さに応じたRGBAカラー</returns>
    DirectX::XMVECTOR CalculateColor(float y)const 
    {
        float t = (y + 1.0f) / 2.0f;

        //0.0fより小さければ0.0f に、1.0fより大きければ1.0f
        t = std::min(1.0f, std::max(0.0f, t));

        //深い青から明るい水色に変わる
        return DirectX::XMVectorLerp(
            DirectX::Colors::DarkBlue.v,
            DirectX::Colors::Cyan.v, t
        );
    }


public:

    /// <summary>
    /// 波のコンストラクタ
    /// </summary>
    /// <param name="deviceResources">デバイスリソース</param>
    Wave(DX::DeviceResources* deviceResources);
    
    /// <summary>
    /// 波の初期化　[継承]
    /// </summary>
    /// <param name="hwnd">ウィンドウハンドル</param>
    /// <param name="width">画面の幅</param>
    /// <param name="height">画面の高さ</param>
    void Initialize(HWND hwnd, int width, int height) override;
    
    /// <summary>
    /// 波の更新　[継承]
    /// </summary>
    /// <param name="deltaTime">前フレームからの経過時間</param>
    void Update(float deltaTime) override;

    /// <summary>
    /// 波の描画　[継承]
    /// </summary>
    /// <param name="context">デバイスコンテキスト</param>
    /// <param name="view">ビュー行列</param>
    /// <param name="proj">プロジェクション行列</param>
    void Render(
        ID3D11DeviceContext* context,
        const DirectX::SimpleMath::Matrix& view, 
        const DirectX::SimpleMath::Matrix& proj) override;

    void OnDeviceLost() override;
    void OnDeviceRestored() override;

    /// <summary>
    /// 波オブジェクトの基準座標を取得
    /// </summary>
    /// <returns>波の座標</returns>
    DirectX::SimpleMath::Vector3 GetPosition() const;

    /// <summary>
    /// 指定された座標 (X, Z) における波の揺れ角度（勾配）を取得
    /// ステージの傾きを渡すためのもの
    /// </summary>
    /// <param name="x">ワールドX座標</param>
    /// <param name="z">ワールドZ座標</param>
    /// <returns>X方向・Z方向の傾き具合</returns>
    DirectX::SimpleMath::Vector2 GetWaveAngle(float x, float z) const;

    //波の高さを取得

    /// <summary>
    /// 指定された座標 (X, Z) における波の現在高さを取得
    /// </summary>
    /// <param name="x">ワールドX座標</param>
    /// <param name="z">ワールドZ座標</param>
    /// <returns>波のY座標（高さ）</returns>
    float GetHeight(float x, float z) const;
    
    /// <summary>
    /// 波の更新
    /// </summary>
    void UpdateWaveVertices();

    /// <summary>
    /// モードを反転させる
    /// </summary>
    void ToggleDotMode() { m_isDotMode = !m_isDotMode; }

private:

    /// <summary>
    /// デバイス依存のリソースを作成する
    /// </summary>
    void CreateDeviceResources();

    /// <summary>
    /// ウィンドウサイズに依存するリソースを作成・更新する
    /// </summary>
    /// <param name="width">ウィンドウの幅</param>
    /// <param name="height">ウィンドウの高さ</param>
    void CreateWindowSizeResources(int width, int height);

    /// <summary>
    /// キューブのバッファを作る専用
    /// </summary>
    void CreateCubeBuffer();

private:

    //リソース
    DX::DeviceResources* m_deviceResources;

    //  射影行列
    DirectX::SimpleMath::Matrix m_proj;

    std::unique_ptr<DirectX::CommonStates> m_states;

    //座標
    DirectX::SimpleMath::Vector3 m_position;

    std::unique_ptr<DirectX::EffectFactory> m_effectFactory;

    //波の生成用で使うやつ
    std::vector<WaveVertex> m_waveVertices;

    //揺れの時間
    float m_time = 0.0f;

    //PrimitiveBatchで描画用で使うやつ
    std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>> m_batch;

    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;


    //頂点バッファ
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
    //インデックスバッファ
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
    //描画する頂点の総数
    UINT m_indexCount = 0;

    //キューブ用のバッファ
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_cubeVertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_cubeIndexBuffer;

    //自作シェーダを扱うためのポインタ
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;

    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
    //定数バッファ
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer;
    
    //波の切り替え
    bool m_isDotMode = false;

};
