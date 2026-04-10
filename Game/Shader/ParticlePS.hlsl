#include "Particle.hlsli"

Texture2D tex : register(t0);
Texture2D tex2 : register(t1);
SamplerState samLinear : register(s0);

float4 main(PS_INPUT input) : SV_TARGET
{
    ////	‰æ‘œ•\Ž¦
    //float4 output = tex.Sample(samLinear, input.Tex);
    
    //return output;
    
    float4 color = tex.Sample(samLinear, input.Tex);
    
    color *= input.Color;
    
    return color;
    
    
}