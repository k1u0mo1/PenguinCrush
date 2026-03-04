#include "Rain.hlsli"

PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT) 0;

    // --- 設定 ---
    float range = 60.0f; // 雨を降らせる範囲（20m四方）
    float speed = params.x; // C++から送られてきた速度 (例: 15.0)

    // --- 動きの計算 ---
    // 1. 基本的な落下移動 (Y軸)
    float moveY = time.x * speed;
    
    // 2. カメラに追従させる計算 (重要!)
    // 現在の座標 = 初期配置(input.Pos) - 移動量
    float3 currentPos = input.Pos;
    currentPos.y -= moveY;

    // 3. ループ処理 (ここが魔法です)
    // カメラとの相対位置を計算
    float3 relativePos = currentPos - cameraPos.xyz;

    // 「範囲(range)で割った余り」を使うことで、無限にループさせる
    // frac() は小数を返す関数なので、0.0～1.0 になります
    // そこに range を掛けて元のスケールに戻し、中心を調整します
    relativePos.x = (frac(relativePos.x / range) - 0.5f) * range;
    relativePos.y = (frac(relativePos.y / range) - 0.5f) * range;
    relativePos.z = (frac(relativePos.z / range) - 0.5f) * range;

    // 最終的な世界座標 = カメラ位置 + ループさせた相対位置
    float3 worldPos = cameraPos.xyz + relativePos;

    // 次のステージ(GS)へ渡すために、まずは座標だけセット
    output.Pos = float4(worldPos, 1.0f);
    
    // サイズ情報などを色やテクスチャ座標の空き地に詰めて送る
    output.Tex = input.Size;

    return output;
}