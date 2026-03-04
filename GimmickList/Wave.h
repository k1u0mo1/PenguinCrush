// Wave.h
#pragma once

#include "pch.h"

#include "Common/DeviceResources.h"
#include "Common/StepTimer.h"
#include "Effects.h"

//継承/////////////////////////
#include "Library/IGimmick.h"
///////////////////////////////

#include <vector>
//Wave（波）のクラス   [IGimmickに継承]

/// <summary>
/// 水面の波を動的に生成・描画し、物理的な高さや傾きを提供するギミッククラス
/// </summary>
class Wave : public IGimmick, public DX::IDeviceNotify
{
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
        //ランダム用(頂点の振幅)
        float amplitude = 0.0f;

    };

    //形状と解像度に関する定数
    //高さ
    static constexpr int GRID_WIDTH = 100;
    static constexpr int GRID_HEIGHT = 100;

    //間隔
    static constexpr float GRID_SPACING = 0.5f;

    //波の物理挙動に関する定数
    //波の細かさ
    static constexpr float WAVE_FREQUENCY = 0.4f;
    //時間経過の速さ
    static constexpr float WAVE_SPEED = 0.02f;
    //波の高さ tukawanaikamo
    static constexpr float WAVE_AMPLIYUDE = 0.5f;

    //波を配置する高さ
    static constexpr float WORLD_Y = -2.0f;
    //波のスケール
    static constexpr float WORLD_SCALE = 10.0f;

    //ヘルパー関数

    /// <summary>
    /// 波の高さ Y を計算
    /// UpdateWaveVerticesとGetHeightで使用
    /// </summary>
    /// <param name="x">座標X</param>
    /// <param name="z">座標Y</param>
    /// <param name="time">時間</param>
    /// <param name="individualAmp"></param>
    /// <returns></returns>
    float CalculateHeight(
        float x, float z,
        float time,
        float individualAmp)const 
    {
        float wave =
            sinf(x * WAVE_FREQUENCY + time)
            + cosf(z * WAVE_FREQUENCY + time);

        return wave * WAVE_AMPLIYUDE * individualAmp;
    }

    /// <summary>
    /// 高さから色を計算
    /// </summary>
    /// <param name="y"></param>
    /// <returns></returns>
    DirectX::XMVECTOR CalculateColor(float y)const 
    {
        float t = (y + 1.0f) / 2.0f;

        //0.0fより小さければ0.0f に、1.0fより大きければ1.0f
        if (t < 0.0f)t = 0.0f;
        if (t > 1.0f)t = 1.0f;

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
    /// <param name="deviceResources"></param>
    Wave(DX::DeviceResources* deviceResources);
    
    /// <summary>
    /// 波の初期化　[継承]
    /// </summary>
    /// <param name="hwnd"></param>
    /// <param name="width"></param>
    /// <param name="height"></param>
    void Initialize(HWND hwnd, int width, int height) override;
    
    /// <summary>
    /// 波の更新　[継承]
    /// </summary>
    /// <param name="elapsedTime">前フレームからの経過時間</param>
    void Update(float elapsedTime) override;

    /// <summary>
    /// 波の描画　[継承]
    /// </summary>
    /// <param name="context"></param>
    /// <param name="view"></param>
    /// <param name="proj"></param>
    void Render(
        ID3D11DeviceContext* context,
        const DirectX::SimpleMath::Matrix& view, 
        const DirectX::SimpleMath::Matrix& proj) override;

    void OnDeviceLost() override;
    void OnDeviceRestored() override;

    /// <summary>
    /// 波オブジェクトの基準座標を取得
    /// </summary>
    /// <returns>m_position</returns>
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

private:

    void CreateDeviceResources();

    void CreateWindowSizeResources(int width, int height);

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

    std::unique_ptr<DirectX::BasicEffect> m_effect;

    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
    
private:

    


};
