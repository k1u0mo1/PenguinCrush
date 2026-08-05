
/**
 * @file   BaseParticleEmitter.h
 * @brief  共通データと基底クラス管理を行うクラス
 * @author 國田知睦
 * @date   2026/07/29
 */

#pragma once
#include <SimpleMath.h>
#include <PrimitiveBatch.h>
#include <VertexTypes.h>
#include <vector>

/// <summary>
/// 乱数生成のヘルパー関数を追加
/// </summary>
/// <param name="min">最小値</param>
/// <param name="max">最大値</param>
/// <returns></returns>
inline float GetRandom(float min, float max)
{
	return min + (float)rand() / RAND_MAX * (max - min);
}

/// <summary>
/// パーティクル1粒の情報
/// </summary>
struct  ParticleInfo
{
	//位置
	DirectX::SimpleMath::Vector3 Position;
	//速度
	DirectX::SimpleMath::Vector3 Velocity;
	//色
	DirectX::SimpleMath::Vector4 Color;
	//経過時間
	float Age = 0.0f;
	//寿命
	float Lifetime = 1.0f;
	//サイズ
	float Size = 1.0f;
};

/// <summary>
/// 全てのエフェクトの親となるクラス
/// </summary>
class BaseParticleEmitter
{
protected:
	//エフェクトが持つ粒のリスト
	std::vector<ParticleInfo> m_particles;

	//使用するテクスチャの番号 エフェクトごとに変えるため
	int m_textureIndex = 0;

public:
	/// <summary>
	/// デストラクタ
	/// </summary>
	virtual ~BaseParticleEmitter() = default;

	/// <summary>
	/// 派生クラスで独自の動きを実装させる　更新処理
	/// </summary>
	/// <param name="elapsedTime">前フレームからの経過時間</param>
	virtual void Update(float elapsedTime) = 0;

	/// <summary>
	/// 描画処理　（共通の処理はここで実行）
	/// </summary>
	/// <param name="batch">バッチクラスのポインタ</param>
	virtual void Render(
		DirectX::PrimitiveBatch<DirectX::VertexPositionColorTexture>* batch)
	{
		for (const auto& p : m_particles)
		{
			//寿命に対する現在の経過時間の割合を計算
			float ratio = p.Age / p.Lifetime;
			//経過時間に合わせて徐々に透明にする
			float alpha = 1.0f - ratio;
			if (alpha < 0.0f)
			{
				alpha = 0.0f;
			}
			//パーティクルの基本色に、計算したアルファ値を適用
			DirectX::SimpleMath::Vector4 color = p.Color;
			color.w = alpha;

			//頂点データの作成
			DirectX::VertexPositionColorTexture v(
				p.Position, color, DirectX::SimpleMath::Vector2(p.Size, 0.0f)
			);
			//点として描画バッチに追加
			batch->Draw(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST, &v, 1);
		}
	}

	/// <summary>
	/// エフェクトが終了したかどうかの判断
	/// </summary>
	/// <returns>パーティクルのリストが空ならtrue</returns>
	bool Isdead() const { return m_particles.empty(); }

	/// <summary>
	/// テクスチャの番号を取得する
	/// </summary>
	/// <returns>テクスチャの番号</returns>
	int GetTextureUndex() const { return m_textureIndex; }

};