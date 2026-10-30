
/**
 * @file   WavePS.hlsl
 * @brief  波用のシェーダ
 * @author 國田知睦
 * @date   2026/10/18
 */

#include "Wave.hlsli"

Texture2D tex : register(t0);
Texture2D tex2 : register(t1);
SamplerState samLinear : register(s0);

float4 main(PS_INPUT input) : SV_TARGET
{
    
    //スクリーン空間の微分関数から波の法線を動的生成
    float3 pixelDerivX = ddx(input.WorldPos);
    float3 pixelDerivY = ddy(input.WorldPos);
    float3 worldNormal = normalize(cross(pixelDerivY, pixelDerivX)); //表面の法線ベクトル
    
    //太陽光のパラメータ定義
    float3 lightDir = normalize(float3(0.4f, 0.8f, -0.4f));
    float3 lightColor = float3(1.0f, 0.95f, 0.9f);
    
    //拡散反射
    float NdotL = max(0.2f, dot(worldNormal, lightDir));
    
    //鏡面反射
    float3 viewDir = normalize(-input.WorldPos);//近似カメラ方向
    float3 halfDir = normalize(lightDir + viewDir);
    float NdotH = max(0.0f, dot(worldNormal, halfDir));
    float specular = pow(NdotH, 64.0f) * 1.5f;//強いハイライト
    
    //波のベースとなる色　水色～深青
    float heightFactor = saturate((input.WorldPos.y + 1.5f) / 3.0f);
    //波の色をまとめる
    float3 baseColor = lerp(deepColor.rgb, shallowColor.rgb, heightFactor);
    
    //白波　泡の表現
    float foamFactor = pow(heightFactor, 4.0f);
    float3 foamColor = float3(0.95f, 0.98f, 1.0f);
    
    //最終のカラー合成
    float3 finalColor = baseColor * NdotL * lightColor + (foamColor * foamFactor) + (specular * lightColor);
    
    return float4(finalColor, 1.0f);
    
}