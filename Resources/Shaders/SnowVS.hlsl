#include "Snow.hlsli"

PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT) 0;

    //設定
    float range = 100.0f; // 雪の降る範囲の広さ
    float height = 50.0f; // 高さ
    float speed = params.x;

    // まず雪の基本位置を計算
    float3 basePos = input.Pos;
    basePos.y -= time.x * speed;

    // カメラから見た相対位置（距離）を計算する
    float3 distFromCam = basePos - cameraPos.xyz;

    //その距離を range で割った余り（ループ）にする
    // X軸のループ（カメラ中心に配置）
    float moveX = (frac(distFromCam.x / range + 0.5f) - 0.5f) * range;
    
    // Z軸のループ（カメラ中心に配置）
    float moveZ = (frac(distFromCam.z / range + 0.5f) - 0.5f) * range;
    
    // Y軸のループ（高さ制限）
    float moveY = (frac(distFromCam.y / height + 0.5f) - 0.5f) * height;


    // ループさせた相対位置を、現在のカメラ位置に足し戻す
    float3 worldPos = cameraPos.xyz + float3(moveX, moveY, moveZ);


    // 揺らぎなどの演出（座標確定後に足す）
    worldPos.x += sin(time.x + worldPos.y * 0.1f) * 0.5f;
    worldPos.z += cos(time.x * 0.5f + worldPos.y * 0.1f) * 0.5f;


    // 出力
    output.Pos = float4(worldPos, 1.0f);
    output.WorldPos = worldPos; // GS/PSで使う用
    
    
    // サイズなどを渡す場合は適宜
    output.Tex = input.Size;
    
    return output;
}