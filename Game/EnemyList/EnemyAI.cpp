
/**
 * @file   EnemyAI.cpp
 * @brief  敵AIの動きの管理を行うクラス
 * @author 國田知睦
 * @date   2026/10/25
 */

#include "pch.h"
#include "EnemyAI.h"

 //----------------------------------------------------------
 // 敵の行動処理
 //----------------------------------------------------------

void EnemyAI::UpdateAI(
	float dt, 
	const DirectX::SimpleMath::Vector3& currentPos, 
	const DirectX::SimpleMath::Vector3& playerPos, 
	Stage* stage,
	float speed,
	DirectX::SimpleMath::Vector3& outVelocity,
	float& outRotationY)
{
	UNREFERENCED_PARAMETER(dt);
	
	DirectX::SimpleMath::Vector3 forward = playerPos - currentPos;
	forward.y = 0.0f;

	//AI
	if (forward.LengthSquared() > VECTOR_EPSILON)
	{
		forward.Normalize();

		//穴を避けるための先読み処理
		//何メートル先の地面のチェックするか？
		float checkDistance = FALL_CHECK_DISTANC + speed;
		DirectX::SimpleMath::Vector3 nextPos = currentPos + forward * FALL_CHECK_DISTANC;

		//予測位置の足場の高さを取得
		float groundY = stage->GetGroundHeight(nextPos.x, nextPos.z);

		//足場の高さが一定以下なら穴と判断して回避行動
		if (groundY <= STAGE_BOUNDARY_Y)
		{

			//左右の回避ベクトルを計算
			DirectX::SimpleMath::Vector3 rightDir( forward.z, 0.0f, -forward.x);
			DirectX::SimpleMath::Vector3 leftDir(-forward.z, 0.0f, forward.x);

			//左右それぞれの先読み位置
			DirectX::SimpleMath::Vector3 rightPos = currentPos + rightDir * checkDistance;
			DirectX::SimpleMath::Vector3 leftPos  = currentPos + leftDir  * checkDistance;

			
			float rightGroundY = stage ? stage->GetGroundHeight(rightPos.x, rightPos.z) : STAGE_BOUNDARY_Y;
			float leftGroundY  = stage ? stage->GetGroundHeight(leftPos. x, leftPos.z) : STAGE_BOUNDARY_Y;

			bool isRightSafe = (rightGroundY > STAGE_BOUNDARY_Y);
			bool isLeftSafe = (leftGroundY > STAGE_BOUNDARY_Y);

			
			if (isRightSafe)
			{
				forward = rightDir;
			}
			else if (isLeftSafe)
			{
				forward = leftDir;
			}
			else
			{
				//左右が無理なら後退
				forward = -forward;
			}
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
