#include "Wave.hlsli"

//波の見た目を作る頂点シェーダ

//WaveVS.hlsl

PS_INPUT main(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;

	//波の見た目を作るためのパラメータ
    float WAVE_FREQUENCY = 0.15f;
    //波の高さ
    float WAVE_AMPLITUDE = 1.5f;
    
    //計算した高さをYに適用
    float4 localPos = float4(input.Pos.x, input.Pos.y, input.Pos.z, 1.0f);
    
    //キューブの波
    if (isCubeMode == 1)
    {
        //グリッドの幅
        int gridWidth = 200;
        //キューブの大きさ
        float cubeScale = 5.0f;
        //キューブ同士のスペース 少し空ける
        float spacing = cubeScale * 0.25f;
        
        
        //グリッドのどこにいるのかを計算
        int gridX = input.instanceID % gridWidth;
        int gridZ = input.instanceID / gridWidth;
        
        //中心を（０，０）にするためのオフセット
        float offsetX = (gridX - (gridWidth * 0.5f)) * spacing;
        float offsetZ = (gridZ - (gridWidth * 0.5f)) * spacing;
        
        //１つ１つのキューブを特定の位置に移動
        localPos.x = input.Pos.x * cubeScale;
        localPos.y = input.Pos.y * cubeScale;
        localPos.z = input.Pos.z * cubeScale;

        //オフセットを加える
        localPos.x += offsetX;
        localPos.z += offsetZ;
        
        //波１　ベースとなる揺れが大きくゆったり
        float waveSlowly = sin(offsetX * WAVE_FREQUENCY + time * 0.3f);
        //波２　少し細かく動きが速い
        float waveQuick = cos(offsetZ * WAVE_AMPLITUDE * 0.25f + time * 1.2f) * 0.5f;
        //波３　斜めの方向へ細かい波
        float waveSlanting = sin((offsetX + offsetZ) * 1.0f + time * 1.0f) * 0.25f;
        //波４　逆斜めの方向の細かい波
        float waveReverse = cos((offsetX - offsetZ) * 0.25f + time * 0.5f) * 0.5f;
	
        //波の合成
        float wave = waveSlowly + waveQuick + waveSlanting - waveReverse;
        float y = wave * WAVE_AMPLITUDE;
        
        localPos.y += y;
    }
    //通常の波
    else
    {
        //CPUのCalculateHeightと同じ計算をGPUでする
        float localX = input.Pos.x;
        float localZ = input.Pos.z;
	
        //波別々
        float waveSlowly = sin(localX * WAVE_FREQUENCY + time * 1.0f);
        float waveQuick = cos(localZ * WAVE_AMPLITUDE * 1.5f + time * 1.2f) * 0.5f;
        float waveSlanting = sin((localX + localZ) * 1.0f + time * 1.0f) * 0.25f;
        float waveReverse = cos((localX - localZ) * 0.25f + time * 0.5f) * 0.5f;
	
        //波の合成
        float wave = waveSlowly + waveQuick + (waveSlanting - waveReverse);
        float y = wave * WAVE_AMPLITUDE;
        
        localPos.y += y;
    }
    
    //以下はどちらも使うもの
    
    //空間の変換
    float4 worldPos = mul(localPos, matWorld);
    float4 viewPos  = mul(worldPos,  matView);
    output.Pos      = mul(viewPos,   matProj);
	
    //ピクセルシェーダに情報を渡す
    output.WorldPos = localPos.xyz;
    output.Color = input.Color;
    
	return output;
}