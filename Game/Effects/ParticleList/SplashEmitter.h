
/**
 * @file   SplashEmitter.h
 * @brief  落下した際の水しぶき専用　パーティクルクラス
 * @author 國田知睦
 * @date   2026/07/29
 */

#pragma once
#include "Game/Effects/BaseParticleEmitter.h"
#include <SimpleMath.h>

/// <summary>
/// 水しぶき専用のエミッタークラス
/// </summary>
class SplashEmitter : public BaseParticleEmitter
{
private:

	//基本設定//
	//サイズ
	static constexpr float EFFECT_SIZE = 1.5f;
	//カラー
	static constexpr DirectX::SimpleMath::Vector4 BASE_COLOR = { 0.8f,0.9f,1.0f,1.0f };
	//寿命の最小値・最大値
	static constexpr float LIFETIME_MIN = 0.5f;
	static constexpr float LIFETIME_MAX = 3.0f;

	//発生座標を少しばらつかせる
	static constexpr float POSITION_MIN = -0.5f;
	static constexpr float POSITION_MAX =  0.5f;

	//円形に広がる 速さ
	static constexpr float CIRCULAR_SPEED_MIN = 5.0f;
	static constexpr float CIRCULAR_SPEED_MAX = 30.0f;
	//円形に広がる 半径
	static constexpr float CIRCULAR_RADIUS_MIN = 0.0f;
	static constexpr float CIRCULAR_RADIUS_MAX = 5.0f;
	//重力
	static constexpr float VELOCITY_DECAY = -19.6f;

public:

	/// <summary>
	/// コンストラク　発生処理
	/// </summary>
	/// <param name="position">発生座標</param>
	/// <param name="count">エフェクトの数</param>
	SplashEmitter(DirectX::SimpleMath::Vector3 position, int count);

	/// <summary>
	/// 更新処理
	/// </summary>
	/// <param name="elapsedTime">前フレームからの経過時間</param>
	void Update(float elapsedTime) override;
};