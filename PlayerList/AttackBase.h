
//攻撃の基底クラス-------------------------------

#pragma once
#include <DirectXMath.h>
#include <memory>
#include <SimpleMath.h>
#include <d3d11.h>

class ModelCollision;

class AttackBase
{
public: 
	virtual ~AttackBase() = default;

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

	//ノックバックパワーを設定する関数を追加
	void SetKnockbackPower(float power) { m_knockbackPower = power; }

	//衝突情報を取得
	virtual ModelCollision* GetCollision() const = 0;

	//ワールド座標を取得
	virtual DirectX::SimpleMath::Vector3 GetPosition() const = 0;

	DirectX::SimpleMath::Vector3 GetForward()const { return m_forward; }


	//攻撃力を取得する純粋仮想関数
	virtual float GetDamage() const = 0;

protected:

	//派生用
	void SetForward(const DirectX::SimpleMath::Vector3& f) { m_forward = f; }

private:

	//ノックバックを保持
	float m_knockbackPower = 0.0f;

	//ベクトル
	DirectX::SimpleMath::Vector3 m_forward = DirectX::SimpleMath::Vector3::UnitZ;
};
