
//攻撃の基底クラス-------------------------------

#pragma once
#include <DirectXMath.h>
#include <memory>
#include <SimpleMath.h>
#include <d3d11.h>

class ModelCollision;

class EnemyAttackBase
{
public: 
	virtual ~EnemyAttackBase() = default;

	//更新　弾の移動　処置
	virtual void Update(float dt) = 0;

	virtual void Render(
		ID3D11DeviceContext* context,
		const DirectX::SimpleMath::Matrix& view,
		const DirectX::SimpleMath::Matrix& proj
	) = 0;

	// 死んだかどうか
	virtual bool IsDead() const = 0;

	//攻撃を無効化する関数
	virtual void SetDead() = 0;

	virtual float GetKnockbackPower() const = 0;

	//衝突情報を取得
	virtual ModelCollision* GetCollision() const = 0;

	//ワールド座標を取得
	virtual DirectX::SimpleMath::Vector3 GetPosition() const = 0;

	//進行方向を取得
	virtual DirectX::SimpleMath::Vector3 GetForward() const = 0;

	//攻撃力を取得する純粋仮想関数
	virtual float GetDamage() const = 0;

};
