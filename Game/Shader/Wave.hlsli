
//Wave.hlsli

//C++から毎フレーム送られてくるデータ
cbuffer ConstBuffer	: register(b0)
{
	matrix matWorld;
	matrix matView;
	matrix matProj;
    float time; //波を動かす用の時間
    int isCubeMode;//1:キューブ　0:通常の波
    float2 padding;
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