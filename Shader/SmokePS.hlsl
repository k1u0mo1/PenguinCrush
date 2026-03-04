#include "Smoke.hlsli"

Texture2D tex : register(t0);
Texture2D tex2 : register(t1);
SamplerState samLinear : register(s0);

float4 main(PS_INPUT input) : SV_TARGET
{
    
    //中心からの距離を計算
    float2 center = float2(0.5f, 0.5f);
    float dist = distance(input.Tex, center);
    
    //円形のグラデーション
    float alpha = 1.0f - smoothstep(0.0f, 0.5f, dist);
    
    //テクスチャの色を取得
    float4 texColor = tex.Sample(samLinear, input.Tex);
    
    //出力
    float4 output;
    
    //頂点カラーを乗算
    output.rgb = texColor.rgb + input.Color.rgb;
    
    output.a = texColor.a * alpha * input.Color.a * (1.0f - time.x);
    
    clip(output.a - 0.01f);
    
    return output;
    
    
}