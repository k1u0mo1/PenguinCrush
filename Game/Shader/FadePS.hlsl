
Texture2D<float4> Texture : register(t0);
sampler TextureSampler : register(s0);

Texture2D<float4> MaskTexture : register(t1);

cbuffer Parameters : register(b1)
{
    float Rate;
};

float4 main(float4 color : COLOR0, float2 texCoord : TEXCOORD0) : SV_Target0
{
    float4 col = Texture.Sample(TextureSampler, texCoord) * color;
    float4 mask = MaskTexture.Sample(TextureSampler, texCoord);

    // 横方向にワイプ
//    float rate = saturate((texCoord.x - 1.0f) + Rate * 2.0f);
//    float rate = saturate(-texCoord.x + Rate * 2.0f);

    // マスク用テクスチャを使用
    //float rate = saturate((mask.x - 1.0f) + Rate * 2.0f);
    float rate = saturate(-mask.x + Rate * 2.0f);
    
    
    //市松模様-------------------------------
    
    float2 uv = texCoord;
    
    uv.x *= 16.0f;
    uv.y *= 9.0f;
    
    //整数にする
    int x = trunc(uv.x) % 2; //(0~16)偶数か
    int y = trunc(uv.y) % 2; //(0~9)
    
    //排他的論理和( ^ )
    int r = x ^ y; //0 or 1 
    
    //X－＞X
    //float alpha = lerp(Rate * 4 - frac(uv.x), Rate * 4 - frac(uv.x) - 2.0f, r);
    
    //↑↓同じ　   ↓処理速度が上がる
    float alpha = Rate * 4 - frac(uv.x) - r * 2.0f;
    
    
    //---------------------------------------
    
    //return float4(col.rgb, alpha);//市松模様
    return float4(col.rgb, rate);//シェード
}
