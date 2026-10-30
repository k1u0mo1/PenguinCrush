
/**
 * @file   Wave.h
 * @brief  水面の波を動的に生成・描画し、物理的な高さや傾きを提供するギミッククラス
 * @author 國田知睦
 * @date   2026/10/19
 */

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
    //ワールド行列
    DirectX::SimpleMath::Matrix matWorld;
    //ビュー行列
    DirectX::SimpleMath::Matrix matView;
    //射影行列
    DirectX::SimpleMath::Matrix matProj;

    //時間
    float time;
    //波の高さ
    int isCubeMode;

    //波の細かさ
    float waveFrequency;
    //波の高さ
    float waveAmplitude;
    //全体の波の速さ
    float waveSpeed;
	//波のとがり具合
    float waveSharpness;

    //キューブの大きさ
    float cubeScale;
    //グリッドの幅
    int gridWidth;

    //カラー 深い海の色
    DirectX::SimpleMath::Vector4 deepColor;
    //カラー 波の頂点・浅い色
    DirectX::SimpleMath::Vector4 shallowColor;

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

    //視野角
    static constexpr float CAM_FOV = 45.0f;
    //描画開始距離
    static constexpr float CAM_NEAR = 0.1f;
    //描画終了距離
    static constexpr float CAM_FAR = 1000.0f;

    //立方体のインデックスの数
    static constexpr UINT CUBE_INDEX_COUNT = 36;
    //立方体の描画個数
    static constexpr UINT CUBE_INSTANCE_COUNT = 40000;

    //立方体のサイズ
    static constexpr float CUDE_SIZE = 0.2f;
    //立方体の頂点数
    static constexpr UINT CUBE_SIZE = 8;

    ////波1の速度
    //static constexpr float WAVE1_SPEED = 1.0f;

    ////波2の細かさ
    //static constexpr float WAVE2_FREQ_SCALE = 1.5f;
    ////波2の速度
    //static constexpr float WAVE2_SPEED = 1.2f;
    ////波2の高さ
    //static constexpr float WAVE2_AMP_SCALE = 0.5f;

    ////波3の細かさ
    //static constexpr float WAVE3_FREQ_SCALE = 0.5f;   
    ////波3の動く速度
    //static constexpr float WAVE3_SPEED = 1.0f;   
    ////波3の高さ
    //static constexpr float WAVE3_AMP_SCALE = 0.25f;  

    ////波4の細かさ
    //static constexpr float WAVE4_FREQ_SCALE = 0.25f;  
    ////波4の動く速度
    //static constexpr float WAVE4_SPEED = 0.5f;  
    ////波4の高さ（振幅）の倍率
    //static constexpr float WAVE4_AMP_SCALE = 0.5f;   

    //波の傾き計算用のオフセット
    static constexpr float GRADIENT_CALC_OFFSET = 0.1f;
    //波の傾き計算用の分母
    static constexpr float GRADIENT_CALC_DENOMINATOR = GRADIENT_CALC_OFFSET * 2.0f;
    //波の傾き計算用の半分の係数
    static constexpr float HALF_COEFFICIENT = 0.5f;

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
    static constexpr int GRID_WIDTH = 200;
    static constexpr int GRID_HEIGHT = 200;
    //間隔
    static constexpr float GRID_SPACING = 0.5f;

    
    
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

        float t = time * m_uiSpeed;
        DirectX::SimpleMath::Vector2 P(localX, localZ);
        //float2 P(localX, localZ);
        
        float totalY = 0.0f;

        //GerstnerWaveのY成分を加算するラムダ式
        auto AddGersrnerY = [&](
            DirectX::SimpleMath::Vector2 dir,
            float steepness, float wavelength,
            float speed)
            {
                float k = 2.0f * 3.1415f / wavelength;
                float c = sqrtf(9.8f / k);
                DirectX::SimpleMath::Vector2 d = dir;
                d.Normalize();
                float f = k * (d.x * P.x + d.y * P.y - c * t * speed);
                float a = steepness / k;

                totalY += a * sinf(f);
            };

        //４つの波を合成
        AddGersrnerY(DirectX::SimpleMath::Vector2(0.866f, 0.500f), m_uiSharpness * 0.15f, 20.0f / m_uiFrequency, m_uiSpeed);
        AddGersrnerY(DirectX::SimpleMath::Vector2(-0.707f, -0.707f), m_uiSharpness * 0.12f, 12.0f / m_uiFrequency, m_uiSpeed * 1.2f);
        AddGersrnerY(DirectX::SimpleMath::Vector2(-0.500f, 0.866f), m_uiSharpness * 0.08f, 8.0f / m_uiFrequency, m_uiSpeed * 1.5f);
        AddGersrnerY(DirectX::SimpleMath::Vector2(0.965f, -0.258f), m_uiSharpness * 0.05f, 4.0f / m_uiFrequency, m_uiSpeed * 2.0f);
       
        return totalY * m_uiAmplitude;

	
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
    //void UpdateWaveVertices();

    /// <summary>
    /// モードを反転させる
    /// </summary>
    void ToggleDotMode() { m_isDotMode = !m_isDotMode; }

    /// <summary>
    /// UIの表示・非表示
    /// </summary>
    void ToggleUI() { m_showUI = !m_showUI; }

    /// <summary>
    /// ImGui パラメータ調整用のUI描画
    /// </summary>
    void RenderUI();

    /// <summary>
    /// 波のパラメータテキストファイルに保存する
    /// </summary>
    /// <param name="filePath">保存先のファイルパス</param>
    /// <returns></returns>
    bool SaveParameters(const std::string& filePath = "wave_config.json") const;

    /// <summary>
    /// 波のパラメータテキストファイルから読み込み
    /// </summary>
    /// <param name="filePath">読み込み元のファイルパス</param>
    /// <returns></returns>
    bool LoadParameters(const std::string& filePath = "wave_config.json");

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
    //射影行列
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

    //デバッグの切り替え
    bool m_showUI = false;

    //UIで調整するパラメータ変数
    float m_uiFrequency = 0.25f; //波の細かさ
    float m_uiAmplitude = 3.5f;  //波の高さ
    float m_uiSpeed     = 1.2f;  //波の速さ
    float m_uiSharpness = 2.2f;  //波の尖り具合

    float m_waterColorDeep[4] = { 0.0f,0.2f,0.5f,1.0f };//深い色
    float m_waterColorShallow[4] = { 0.3f,0.8f,1.0f,1.0f };//高い色

    float m_uiCubeScale = 3.0f;//キューブの拡大率
};
