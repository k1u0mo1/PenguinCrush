
/**
 * @file   RainPS.hlsl
 * @brief  雨用のシェーダ
 * @author 國田知睦
 * @date   2026/07/06
 */

#include "Rain.hlsli"

Texture2D tex : register(t0);
Texture2D tex2 : register(t1);
SamplerState samLinear : register(s0);

float4 main(PS_INPUT input) : SV_TARGET
{
    return input.Color;
}