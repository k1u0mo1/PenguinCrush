
/**
 * @file   DashEmitter.h
 * @brief  落下した際の水しぶき専用　パーティクルクラス
 * @author 國田知睦
 * @date   2026/07/15
 */

#pragma once
#include "Game/Effects/BaseParticleEmitter.h"
#include <SimpleMath.h>

/// <summary>
/// 水しぶき専用のエミッタークラス
/// </summary>
class DashEmitter : public BaseParticleEmitter
{
private:

	//基本設定//
	//サイズ
	static constexpr float EFFECT_SIZE = 0.1f;
	//カラー
	static constexpr DirectX::SimpleMath::Vector4 BASE_COLOR = { 1.0f, 1.0f, 1.0f, 0.0f };
	//寿命の最小値・最大値
	static constexpr float LIFETIME_MIN = 0.3f;
	static constexpr float LIFETIME_MAX = 0.8f;

	//発生座標を少しばらつかせる
	static constexpr float POSITION_XZ_MIN = -0.3f;
	static constexpr float POSITION_XZ_MAX =  0.3f;
	static constexpr float POSITION_Y_MIN  =  0.0f;
	static constexpr float POSITION_Y_MAX  =  0.5f;

	//速度を設定
	static constexpr float VELOCITY_XZ_MIN = -1.0f;
	static constexpr float VELOCITY_XZ_MAX =  1.0f;
	static constexpr float VELOCITY_Y_MIN = 1.0f;
	static constexpr float VELOCITY_Y_MAX = 2.0f;
	//重力
	static constexpr float VELOCITY_DECAY = -19.6f;

public:

	/// <summary>
	/// コンストラク　発生処理
	/// </summary>
	/// <param name="position">発生座標</param>
	/// <param name="count">エフェクトの数</param>
	DashEmitter(DirectX::SimpleMath::Vector3 position, int count);

	/// <summary>
	/// 更新処理
	/// </summary>
	/// <param name="elapsedTime">前フレームからの経過時間</param>
	void Update(float elapsedTime) override;
};