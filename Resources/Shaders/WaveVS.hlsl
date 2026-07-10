
/**
 * @file   WaveVS.hlsl
 * @brief  波用のシェーダ
 * @author 國田知睦
 * @date   2026/07/06
 */

#include "Wave.hlsli"

//波の見た目を作る頂点シェーダ

//シェーダ内だけで使用する定数//ーーーーーーーーーーーーーー
//キューブの隙間
static const float CUBE_SPACING_RATIO = 0.01f;
//キューブの見た目の大きさを調整
static const float CUBE_SIZE_CORRECTION = 2.5f;

//-------------------------------------------------------------
//共通の波を合成して高さを返す処理
//-------------------------------------------------------------
float CalculateWave(float2 pos, float currentInternalTime, float frequency)
{
    //各波の進む方向（２Dベクトル）//
    //真右から左
    float2 dir1 = float2( 0.955f, 0.296f);
    //手前から奥
    float2 dir2 = float2(-0.422f, 0.906f);
    //斜め45度
    float2 dir3 = float2( 0.656f, -0.754f);
    //少しずらした斜め
    float2 dir4 = float2(-0.819f, -0.573f);
    
    //リアルな法則に合わせた４つの波の合成
    float wave1 = sin(dot(pos, dir1) * frequency * 1.000f + currentInternalTime * 0.431f);
    float wave2 = cos(dot(pos, dir2) * frequency * 1.732f + currentInternalTime * 0.913f + 1.2f);
    float wave3 = sin(dot(pos, dir3) * frequency * 2.618f + currentInternalTime * 1.547f + 3.7f);
    float wave4 = cos(dot(pos, dir4) * frequency * 4.131f + currentInternalTime * 2.399f + 5.1f);
    
    return (wave1 * 0.50f) + (wave2 * 0.30f) + (wave3 * 0.14f) + (wave4 * 0.06f);
}


//-------------------------------------------------------------
//メイン処理
//-------------------------------------------------------------
PS_INPUT main(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;

    //計算した高さをYに適用
    float4 localPos = float4(input.Pos.x, input.Pos.y, input.Pos.z, 1.0f);
    //timeに全体の速度を掛けておく
    float currentInternalTime = time * waveSpeed;
    
    //波の計算で使う「平面座標」を入れる共通のもの
    float2 waveXZ = float2(0.0f, 0.0f);
    
    //キューブの波
    if (isCubeMode == 1)
    {
        //１つ１つのキューブを特定の位置に移動
        localPos.x = input.Pos.x * cubeScale * CUBE_SIZE_CORRECTION;
        localPos.y = input.Pos.y * cubeScale * CUBE_SIZE_CORRECTION;
        localPos.z = input.Pos.z * cubeScale * CUBE_SIZE_CORRECTION;
        
        //キューブ同士のスペース 少し空ける
        float spacing = cubeScale * CUBE_SPACING_RATIO;
        //1マスの移動距離
        float stepDistance = cubeScale + spacing;
        
        //グリッドのどこにいるのかを計算
        int gridX = input.instanceID % gridWidth;
        int gridZ = input.instanceID / gridWidth;
        
        // int型を float型に明示的にキャストして計算する
        float fGridX = (float) gridX;
        float fGridZ = (float) gridZ;
        float fGridWidth = (float) gridWidth;

        //中心を（０，０）にするためのオフセット
        float totalGridSize = fGridWidth * stepDistance;
        float offsetX = fGridX * stepDistance - totalGridSize * 0.5f;
        float offsetZ = fGridZ * stepDistance - totalGridSize * 0.5f;
        
        //オフセットを加える
        localPos.x += offsetX;
        localPos.z += offsetZ;
        
        //キューブの波を「オフセット座標」をもとに波の計算
        waveXZ = float2(offsetX, offsetZ);
        
    }
    //通常の波
    else
    {
        //「頂点座標」を元にして波を計算
        waveXZ = float2(input.Pos.x, input.Pos.z);
       
    }
    
    //以下はどちらも使うもの
    
    float wave = CalculateWave(waveXZ, currentInternalTime, waveFrequency);
    localPos.y += wave * waveAmplitude;
    
    //空間の変換
    float4 worldPos = mul(localPos, matWorld);
    float4 viewPos  = mul(worldPos,  matView);
    output.Pos      = mul(viewPos,   matProj);
	
    //ピクセルシェーダに情報を渡す
    output.WorldPos = worldPos.xyz;
    output.Color = input.Color;
    
	return output;
}