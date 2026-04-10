#include "Particle.hlsli"

static const int vnum = 4;

static const float4 offset_array[vnum] =
{
	float4(-1.0f,  1.0f, 0.0f, 0.0f),	//	左上
	float4( 1.0f,  1.0f, 0.0f, 0.0f),	//	右上
	float4(-1.0f, -1.0f, 0.0f, 0.0f),	//	左下
	float4( 1.0f, -1.0f, 0.0f, 0.0f),	//	右下

};

static const float2 uv_offset[vnum] =
{
    float2(0.0f, 0.0f), // 左上
	float2(1.0f, 0.0f), // 右上
	float2(0.0f, 1.0f), // 左下
	float2(1.0f, 1.0f), // 右下
};

[maxvertexcount(vnum)]
void main(
	point PS_INPUT input[1],
	inout TriangleStream< PS_INPUT > output
)
{

    
    
	//----------------------------------------------
    //パーティクル系を同じ向きで見れるようにする
    //----------------------------------------------
	
    float size = input[0].Tex.x;

    // 安全策（もし0が来たらデフォルトサイズにする）
    if (size <= 0.0f)
        size = 1.0f;

    float scale = size * 0.5f;
    
    float4 centerWorld = mul(input[0].Pos, matWorld);
    
    float4 centerView = mul(centerWorld, matView);

    // パーティクルの大きさ調整
    //float scale = 0.5f;

    for (int i = 0; i < vnum; i++)
    {
        PS_INPUT element;

        // ビュー空間上で頂点をずらす（ビルボード処理）
        float4 posView = centerView;
        posView.x += offset_array[i].x * scale;
        posView.y += offset_array[i].y * scale;

        // プロジェクション変換
        element.Pos = mul(posView, matProj);

        // 色とUVはそのまま渡す
        element.Color = input[0].Color;
        element.Tex = uv_offset[i];
        
        output.Append(element);
    }
    output.RestartStrip();
}