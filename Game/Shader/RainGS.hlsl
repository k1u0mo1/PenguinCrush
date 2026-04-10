#include "Rain.hlsli"

[maxvertexcount(4)] // 4頂点出して四角形を作る
void main(point PS_INPUT input[1], inout TriangleStream<PS_INPUT> outputStream)
{
    PS_INPUT output;
    
    // 雨のサイズ設定
    float w = 0.05f * params.y; // 細く
    float h = 1.0f * params.y; // 長く

    // 中心座標
    float3 center = input[0].Pos.xyz;

    // ビルボード計算
    // カメラの「右方向」を取得 (View行列の1行目)
    float3 right = float3(matView._11, matView._21, matView._31);
    
    // 雨は「上方向」はY軸固定のほうが自然に見えます
    float3 up = float3(0, 1, 0);

    // 4つの角を作る
    float3 corners[4];
    corners[0] = center - (right * w) + (up * h); // 左上
    corners[1] = center + (right * w) + (up * h); // 右上
    corners[2] = center - (right * w) - (up * h); // 左下
    corners[3] = center + (right * w) - (up * h); // 右下

    // テクスチャ座標 (UV)
    float2 uvs[4] =
    {
        float2(0, 0), float2(1, 0),
        float2(0, 1), float2(1, 1)
    };

    // 4回ループして頂点を書き出す
    for (int i = 0; i < 4; i++)
    {
        // ワールド座標 -> ビュー・プロジェクション変換
        output.Pos = mul(float4(corners[i], 1.0f), matView);
        output.Pos = mul(output.Pos, matProj);
        
        output.Tex = uvs[i];
        output.Color = float4(0.8f, 0.9f, 1.0f, 0.5f); // 薄い青白、半透明
        
        outputStream.Append(output);
    }
}