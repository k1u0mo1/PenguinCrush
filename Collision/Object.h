
//オブジェクト

#pragma once

#include "SimpleMath.h"


/// <summary>
/// 衝突判定用の座標・回転と、各種判定ボリュームをまとめた構造体
/// 球、AABB、OBB
/// </summary>
struct  Object
{
	//位置
	DirectX::SimpleMath::Vector3 position;

	//回転
	DirectX::SimpleMath::Quaternion rotate;

	//衝突判定（球）
	DirectX::BoundingSphere boundingSphere;

	/// <summary>
	/// 現在の position を適用したワールド空間の球判定を取得
	/// </summary>
	/// <returns>BoundingSphere  tmp</returns>
	DirectX::BoundingSphere GetBoundingShere() const
	{
		DirectX::BoundingSphere tmp(boundingSphere);
		tmp.Center.x += position.x;
		tmp.Center.y += position.y;
		tmp.Center.z += position.z;
		return tmp;
	}

	//衝突判定（AABB）
	DirectX::BoundingBox boundingBox;

	// 衝突判定情報取得関数（AABB）
	DirectX::BoundingBox GetBoundingBox() const
	{
		DirectX::BoundingBox tmp(boundingBox);
		tmp.Center = DirectX::SimpleMath::Vector3::Transform(boundingBox.Center, rotate) + position;
		return tmp;
	}

	// 衝突判定情報取得関数（OBB）
	DirectX::BoundingOrientedBox GetBoundingOrientedBox() const
	{

		DirectX::BoundingOrientedBox tmp;
		DirectX::BoundingOrientedBox::CreateFromBoundingBox(tmp, boundingBox);
		tmp.Center = DirectX::SimpleMath::Vector3::Transform(boundingBox.Center, rotate) + position;
		tmp.Orientation = rotate;
		return tmp;
	}
};


