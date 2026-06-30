
/**
 * @file   EnemyAI.cpp
 * @brief  敵AIの動きの管理を行うクラス
 * @author 國田知睦
 * @date   2026/06/17
 */

#include "pch.h"
#include "EnemyAI.h"

void EnemyAI::UpdateAI(
	float /*dt*/, 
	const DirectX::SimpleMath::Vector3& currentPos, 
	const DirectX::SimpleMath::Vector3& playerPos, 
	Stage* stage,
	float speed,
	DirectX::SimpleMath::Vector3& outVelocity,
	float& outRotationY)
{
	
	DirectX::SimpleMath::Vector3 forward = playerPos - currentPos;
	forward.y = 0.0f;

	//AI
	if (forward.LengthSquared() > VECTOR_EPSILON)
	{
		forward.Normalize();

		//穴を避けるための先読み処理
		//何メートル先の地面のチェックするか？
		DirectX::SimpleMath::Vector3 nextPos = currentPos + forward * FALL_CHECK_DISTANC;

		//予測位置の足場の高さを取得
		float groundY = stage->GetGroundHeight(nextPos.x, nextPos.z);

		//足場の高さが一定以下なら穴と判断して回避行動
		if (groundY <= STAGE_BOUNDARY_Y)
		{
			//プレイヤーから見て左右の方向を計算
			DirectX::SimpleMath::Vector3 avoidDir(forward.z, 0.0f, -forward.x);

			//回り込むを新しい進行方向にする
			forward = avoidDir;
		}

		//最終的な向きベクトルを正規化して角度を計算
		outRotationY = std::atan2(forward.x, forward.z);
		outVelocity = forward * speed;
	}
	else
	{
		outVelocity = DirectX::SimpleMath::Vector3::Zero;
	}
}
