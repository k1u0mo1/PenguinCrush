cbuffer ConstBuffer	: register(b0)
{
    matrix matView;   // ビュー行列
    matrix matProj;   // プロジェクション行列
    float4 time;      // time.x = 経過時間
    float4 cameraPos; // カメラの位置
    float4 params;    // x=速度, y=サイズ係数 (調整用)
};

struct VS_INPUT
{
    float3 Pos : POSITION; // 初期位置 (ランダムなばらつき)
    float2 Size : TEXCOORD; // 個別のサイズ
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
    float4 Color : COLOR;
    
};