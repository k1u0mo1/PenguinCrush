#include "Wave.hlsli"

Texture2D tex : register(t0);
Texture2D tex2 : register(t1);
SamplerState samLinear : register(s0);

float4 main(PS_INPUT input) : SV_TARGET
{
   //CPU‚ÌCalculateColor‚Æ“¯‚¶ŒvŽZ
    float t = (input.WorldPos.y + 1.0f) / 2.0f;
    //0.0 `1.0‚Ì”ÍˆÍ‚ÉŽû‚ß‚é
    t = saturate(t);
    
    //DarkBlue->Cyan‚Ö•âŠÔ
    float3 darkBlue = float3(0.0f, 0.0f, 0.545f);
    float3 cyan = float3(0.0f, 1.0f, 1.0f);
    
    float3 finalColor = lerp(darkBlue, cyan, t);
    
    return float4(finalColor, 1.0f);
    
}