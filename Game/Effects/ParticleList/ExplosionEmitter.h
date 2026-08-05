
/**
 * @file   ExplosionEmitter.h
 * @brief  攻撃が当たった時の爆発専用　パーティクルクラス
 * @author 國田知睦
 * @date   2026/07/15
 */

#pragma once
#include "Game/Effects/BaseParticleEmitter.h"
#include <SimpleMath.h>

/// <summary>
/// 攻撃が当たった時の爆発専用のエミッタークラス
/// </summary>
class ExplosionEmitter : public BaseParticleEmitter
{
private:

	//基本設定//
	//サイズ
	static constexpr float EFFECT_SIZE = 0.05f;
	//カラー
	static constexpr DirectX::SimpleMath::Vector4 BASE_COLOR = { 1.0f,0.5f,0.2f,1.0f };
	//寿命の最小値・最大値
	static constexpr float LIFETIME_MIN = 0.5f;
	static constexpr float LIFETIME_MAX = 1.5f;

	//速度・広がり方
	static constexpr float SPEED_MIN =  5.0f;
	static constexpr float SPEED_MAX = 20.0f;
	//重力
	static constexpr float VELOCITY_DECAY = 1.0f;
	
	//サイズ倍率
	static constexpr float SIZE_MAGNIFICATION_RATE = 1.0f;

public:

	/// <summary>
	/// コンストラク　発生処理
	/// </summary>
	/// <param name="position">発生座標</param>
	/// <param name="count">エフェクトの数</param>
	ExplosionEmitter(DirectX::SimpleMath::Vector3 position, int count);

	/// <summary>
	/// 更新処理
	/// </summary>
	/// <param name="elapsedTime">前フレームからの経過時間</param>
	void Update(float elapsedTime) override;
};