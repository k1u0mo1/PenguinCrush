// SlideBehavior.h
//滑る挙動

#pragma once
#include "pch.h"
#include <DirectXMath.h>
#include <SimpleMath.h>

/// <summary>
/// 滑る挙動のコンポーネント
/// 使用：プレイヤー、敵、魚
/// </summary>
class SlideBehavior
{
private:

	//--------------------------------
	//定数
	//--------------------------------
	
	//加速度
	static constexpr float ACCELERATION_FORCE = 0.5f;

	//摩擦力
	static constexpr float FRICTION_FORCE = 2.0f;

public:

	/// <summary>
	/// オブジェクトの移動とステージの滑りをまとめる計算
	/// </summary>
	/// <param name="position">更新したい座標</param>
	/// <param name="targetVelocity">自発的な移動速度</param>
	/// <param name="slideDir">ステージからの強制スライド量</param>
	/// <param name="elapsedTime">デルタタイム</param>
	void Update(DirectX::SimpleMath::Vector3& position,
		const DirectX::SimpleMath::Vector3& targetVelocity,
		const DirectX::SimpleMath::Vector3& slideDir,
		float elapsedTime);
	
private:
	
	//自発的な移動に対する慣性
	DirectX::SimpleMath::Vector3 m_slidingInertia = DirectX::SimpleMath::Vector3::Zero;

	//傾斜による滑りの慣性
	DirectX::SimpleMath::Vector3 m_currentSlideVelocity = DirectX::SimpleMath::Vector3::Zero;

};
