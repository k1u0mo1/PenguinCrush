#include "Snow.hlsli"

Texture2D tex : register(t0);
Texture2D tex2 : register(t1);
SamplerState samLinear : register(s0);

float4 main(PS_INPUT input) : SV_TARGET
{
    float4 color = tex.Sample(samLinear, input.Tex);

    if (color.a <= 0.01f)
        discard;

    // 雪の場所 と カメラの場所 の距離を測る
    float dist = distance(input.WorldPos, cameraPos.xyz);

    //近く(0.5m以内)をフェードアウト
    float fadeNear = saturate((dist - 0.5f) / 4.5f);

    // 遠く(範囲の限界付近)をフェードアウト
    float rangeRadius = 50.0f; // range / 2.0f
    float fadeFar = 1.0f - saturate((dist - (rangeRadius - 10.0f)) / 10.0f);

    // 両方のフェードを適用
    color.a *= fadeNear * fadeFar;

    return color;
}