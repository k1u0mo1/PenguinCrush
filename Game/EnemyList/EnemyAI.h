
/**
 * @file   EnemyAI.h
 * @brief  敵AIの動きの管理を行うクラス
 * @author 國田知睦
 * @date   2026/06/05
 */

#pragma once
#include "pch.h"
#include "Game/GimmickList/Stage.h"

class EnemyAI
{
private:

	//接地する足場の位置の高さ
	static constexpr float STAGE_BOUNDARY_Y = -4.9f;

	//穴を避ける距離
	static constexpr float FALL_CHECK_DISTANC = 3.0f;

	//微小値
	static constexpr float VECTOR_EPSILON = 0.0001f;

public:

	/// <summary>
	/// コンストラクタ
	/// </summary>
	EnemyAI() = default;

	/// <summary>
	/// デストラクタ
	/// </summary>
	~EnemyAI() = default;

	/// <summary>
	///	敵のAIの更新
	/// </summary>
	/// <param name="dt">デルタタイム</param>
	/// <param name="currentPos">現在の位置</param>
	/// <param name="playerPos">プレイヤーの位置</param>
	/// <param name="stage">ステージ情報</param>
	/// <param name="speed">移動速度</param>
	/// <param name="outVelocity">出力される速度ベクトル</param>
	/// <param name="outRotationY">出力されるY軸回転角度</param>
	void UpdateAI(
		float dt,
		const DirectX::SimpleMath::Vector3& currentPos,
		const DirectX::SimpleMath::Vector3& playerPos,
		Stage* stage,
		float speed,
		DirectX::SimpleMath::Vector3& outVelocity,
		float& outRotationY
	);
};