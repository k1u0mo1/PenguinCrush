
/**
 * @file   ParticlePS.hlsl
 * @brief  エフェクト用のシェーダ
 * @author 國田知睦
 * @date   2026/07/06
 */

#include "Particle.hlsli"

Texture2D tex : register(t0);
Texture2D tex2 : register(t1);
SamplerState samLinear : register(s0);

float4 main(PS_INPUT input) : SV_TARGET
{
    
    float4 color = tex.Sample(samLinear, input.Tex);
    
    color *= input.Color;
    
    return color;   
}