
/**
 * @file   WaveVS.hlsl
 * @brief  波用のシェーダ
 * @author 國田知睦
 * @date   2026/10/16
 */

#include "Wave.hlsli"

//波の見た目を作る頂点シェーダ

//シェーダ内だけで使用する定数//ーーーーーーーーーーーーーー
//キューブの隙間
static const float CUBE_SPACING_RATIO = 0.01f;
//キューブの見た目の大きさを調整
static const float CUBE_SIZE_CORRECTION = 2.5f;

//-------------------------------------------------------------
//Gerstner Wave　単体の計算関数
//-------------------------------------------------------------
void CalculateGerstnerWave(
float2 positionXZ,      //波を計算する位置
float2 waveDirection,   //波の進行方向
float steepness,        //波の尖り具合
float wavelength,       //波の長さ　波と波の距離
float speed,            //移動速度
float time,             //時間
inout float3 offset,    //頂点の変位結果
inout float3 binormal,  //従法線
float3 tangent          //接線
)
{
    //波数（波の密度）
    float waveNumber = 2.0f * 3.1445f / wavelength;
    //相速度（波の移動速度）
    float phaseSpeed = sqrt(9.8f / waveNumber);
    //方向（波の進む向き）
    float2 normalizedDir = normalize(waveDirection);
    //位相（時間経過と座標の角度）
    float phase = waveNumber * (dot(normalizedDir, positionXZ) - phaseSpeed * time * speed);
    //振り幅（波の高さ）
    float amplitude = steepness / waveNumber;
    
    //水平変位（X,Z）と垂直変位（Y）の加算
    offset.x += normalizedDir.x * (amplitude * cos(phase));
    offset.y += amplitude * sin(phase);
    offset.z += normalizedDir.y * (amplitude * cos(phase));
}


//-------------------------------------------------------------
//メイン処理
//-------------------------------------------------------------
PS_INPUT main(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;

    //計算した高さをYに適用
    float4 localPos = float4(input.Pos.x, input.Pos.y, input.Pos.z, 1.0f);
    //timeに全体の速度を掛けておく
    float currentInternalTime = time * waveSpeed;
    
    //波の計算で使う「平面座標」
    float2 waveXZ = (isCubeMode == 1) ? float2(0, 0) : float2(input.Pos.x, input.Pos.z);
    
    //キューブの波
    if (isCubeMode == 1)
    {
        
        //各キューブのグリッド位置
        int gridX = input.instanceID % gridWidth;
        int gridZ = input.instanceID / gridWidth;
        
        float fGridX = (float) gridX;
        float fGridZ = (float) gridZ;
        float fGrdWidth = (float) gridWidth;
        
        
        //通常のメッシュと同じ座標空間を作る
        float GRID_SPACING = 0.5f;
        float worldX = (fGridX - fGrdWidth * 0.5f) * GRID_SPACING;
        float worldZ = (fGridZ - fGrdWidth * 0.5f) * GRID_SPACING;
        
        //キューブ自体の配置とサイズを設定
        localPos.x = input.Pos.x * cubeScale + worldX;
        localPos.y = input.Pos.y * cubeScale;
        localPos.z = input.Pos.z * cubeScale + worldZ;
        
        
        float WAVELENGTH_SCALE = 2.5f;
        
        //通常波と同じスケールの座標を渡す
        waveXZ = float2(worldX, worldZ) / WAVELENGTH_SCALE;
        
        
    }
    
    
    //以下はどちらも使うもの
    
    //４つの異なる方向のGerstner　波の合成
    float3 waveOffset = float3(0, 0, 0);
    float3 dummyBinormal = float3(1, 0, 0);
    float3 dummyTangent = float3(0, 0, 1);
    
    //波１　メインの波　右上からメインの大きな波
    CalculateGerstnerWave(
    waveXZ, 
    float2(0.866f, 0.500f),
    waveSharpness * 0.15f, 
    20.0f / waveFrequency,
    waveSpeed,
    currentInternalTime,
    waveOffset, dummyBinormal,
    dummyTangent
    );
    
    //波２　逆方向からぶつかる衝撃波
    CalculateGerstnerWave(
    waveXZ,
    float2(-0.707f, -0.707f),
    waveSharpness * 0.12f,
    12.0f / waveFrequency,
    waveSpeed*1.2f,
    currentInternalTime,
    waveOffset, dummyBinormal,
    dummyTangent
    );
    
    //波３　交差する波
    CalculateGerstnerWave(
    waveXZ,
    float2(-0.500f, 0.866f),
    waveSharpness * 0.08f,
    8.0f / waveFrequency,
    waveSpeed * 1.5f,
    currentInternalTime,
    waveOffset, dummyBinormal,
    dummyTangent
    );
    
    //波４　細かい波
    CalculateGerstnerWave(
    waveXZ,
    float2(0.965f, -0.258f),
    waveSharpness * 0.05f,
    4.0f / waveFrequency,
    waveSpeed * 2.0f,
    currentInternalTime,
    waveOffset, dummyBinormal,
    dummyTangent
    );
    
    //全体振幅を適応
    localPos.xyz += waveOffset * waveAmplitude;
    
    //空間の変換
    float4 worldPos = mul(localPos, matWorld);
    float4 viewPos  = mul(worldPos,  matView);
    output.Pos      = mul(viewPos,   matProj);
	
    //ピクセルシェーダに情報を渡す
    output.WorldPos = worldPos.xyz;
    output.Color = input.Color;
    
	return output;
}