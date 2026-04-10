//
// SlideBehavior.cpp
//

#include "pch.h"

#include "SlideBehavior.h"

//----------------------------------------------------------
// オブジェクトの移動とステージの滑りをまとめる計算
//----------------------------------------------------------

void SlideBehavior::Update(DirectX::SimpleMath::Vector3& position, const DirectX::SimpleMath::Vector3& targetVelocity, const DirectX::SimpleMath::Vector3& slideDir, float elapsedTime)
{

	//自発的な移動の慣性計算
	float lerpFactor = (targetVelocity.LengthSquared() > 0.01f) ? ACCELERATION_FORCE : FRICTION_FORCE;

	m_slidingInertia = DirectX::SimpleMath::Vector3::Lerp(
		m_slidingInertia,
		targetVelocity,
		lerpFactor * elapsedTime
	);

	//微小の慣性は無くす
	if (targetVelocity.LengthSquared() < 0.001f 
		&& m_slidingInertia.LengthSquared() < 0.01f)
	{
		m_slidingInertia = DirectX::SimpleMath::Vector3::Zero;
	}

	//傾斜の慣性計算
	float slideLerpFactor = (slideDir.LengthSquared() > 0.001f) ? ACCELERATION_FORCE : FRICTION_FORCE;

	m_currentSlideVelocity = DirectX::SimpleMath::Vector3::Lerp(
		m_currentSlideVelocity,
		slideDir,
		slideLerpFactor * elapsedTime
	);

	//微小の慣性は無くす
	if (slideDir.LengthSquared() < 0.001f 
		&& m_currentSlideVelocity.LengthSquared() < 0.01f)
	{
		m_currentSlideVelocity = DirectX::SimpleMath::Vector3::Zero;
	}

	//座標の更新(自身の慣性＋ステージからの滑り)
	position += (m_slidingInertia + m_currentSlideVelocity) * elapsedTime;

}
