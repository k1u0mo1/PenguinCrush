#include "Snow.hlsli"

[maxvertexcount(4)]
void main(point PS_INPUT input[1], inout TriangleStream<PS_INPUT> outputStream)
{
    PS_INPUT output;
    
    // 雪の中心位置を「カメラから見た座標 (View空間)」に変換します
    float4 viewPos = mul(float4(input[0].WorldPos, 1.0f), matView);
    
    
    // サイズ計算 
    float seed = frac(sin(dot(input[0].WorldPos.xyz, float3(12.9898, 78.233, 45.164))));
    float size = 0.2f * params.y * (0.8f + seed * 0.4f);


    //ビルボードを作る
    // 四隅のオフセット（左上、右上、左下、右下）
    float3 offsets[4] =
    {
        float3(-size, size, 0.0f), // 左上
        float3(size, size, 0.0f), // 右上
        float3(-size, -size, 0.0f), // 左下
        float3(size, -size, 0.0f) // 右下
    };

    // UV座標
    float2 uvs[4] =
    {
        float2(0.0f, 0.0f),
        float2(1.0f, 0.0f),
        float2(0.0f, 1.0f),
        float2(1.0f, 1.0f)
    };

    // 頂点生成ループ
    for (int i = 0; i < 4; i++)
    {
        // View空間での頂点位置 = 中心(View) + オフセット
        float3 vPos = viewPos.xyz + offsets[i];
        
        // 最後にプロジェクション変換だけ行う
        output.Pos = mul(float4(vPos, 1.0f), matProj);
        
        output.Tex = uvs[i];
        output.Color = float4(1, 1, 1, 1);
        output.WorldPos = input[0].WorldPos; 
        
        outputStream.Append(output);
    }

    outputStream.RestartStrip();
}