
/**
 * @file   Wave.hlsli
 * @brief  波用のシェーダ
 * @author 國田知睦
 * @date   2026/10/14
 */

//C++から毎フレーム送られてくるデータ
cbuffer ConstBuffer	: register(b0)
{
	matrix matWorld;
	matrix matView;
	matrix matProj;
    
    float time; //波を動かす用の時間
    int isCubeMode;//1:キューブ　0:通常の波  
    
    float waveFrequency;
    float waveAmplitude;
    float waveSpeed;
    float waveSharpness;
    
    float cubeScale;
    int gridWidth;
    
    float4 deepColor; //深い海の色
    float4 shallowColor;//高い色
};

struct VS_INPUT
{
	float3 Pos : POSITION;
	float4 Color: COLOR;
    
    uint instanceID : SV_InstanceID;//何個目のコピー？
};

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float4 Color: COLOR;
    float3 WorldPos : TEXCOORD0;
    
};