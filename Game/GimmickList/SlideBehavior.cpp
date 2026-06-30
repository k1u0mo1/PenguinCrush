/**
 * @file   SlideBehavior.cpp
 * @brief  滑る挙動のコンポーネントクラス
 * @author 國田知睦
 * @date   2026/06/11
 */

#include "pch.h"

#include "SlideBehavior.h"

//----------------------------------------------------------
// オブジェクトの移動とステージの滑りをまとめる計算
//----------------------------------------------------------

void SlideBehavior::Update(DirectX::SimpleMath::Vector3& position, const DirectX::SimpleMath::Vector3& targetVelocity, const DirectX::SimpleMath::Vector3& slideDir, float elapsedTime)
{

	//自発的な移動の慣性計算
	float lerpFactor = (targetVelocity.LengthSquared() > VELOCITY_THRESHOLD) ? ACCELERATION_FORCE : FRICTION_FORCE;

	m_slidingInertia = DirectX::SimpleMath::Vector3::Lerp(
		m_slidingInertia,
		targetVelocity,
		lerpFactor * elapsedTime
	);

	//微小の慣性は無くす
	if (targetVelocity.LengthSquared() < STOP_EPSILON
		&& m_slidingInertia.LengthSquared() < VELOCITY_THRESHOLD)
	{
		m_slidingInertia = DirectX::SimpleMath::Vector3::Zero;
	}

	//傾斜の慣性計算
	float slideLerpFactor = (slideDir.LengthSquared() > STOP_EPSILON) ? ACCELERATION_FORCE : FRICTION_FORCE;

	m_currentSlideVelocity = DirectX::SimpleMath::Vector3::Lerp(
		m_currentSlideVelocity,
		slideDir,
		slideLerpFactor * elapsedTime
	);

	//微小の慣性は無くす
	if (slideDir.LengthSquared() < STOP_EPSILON
		&& m_currentSlideVelocity.LengthSquared() < VELOCITY_THRESHOLD)
	{
		m_currentSlideVelocity = DirectX::SimpleMath::Vector3::Zero;
	}

	//座標の更新(自身の慣性＋ステージからの滑り)
	position += (m_slidingInertia + m_currentSlideVelocity) * elapsedTime;

}
