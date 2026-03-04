#pragma once

#include "PlayerList/AttackBase.h"
#include <Model.h>
#include <SimpleMath.h>
#include <memory>
#include <Collision/ModelCollision.h>

class BulletAttack : public AttackBase
{
public:

	//コンストラクタ
	BulletAttack(
		const DirectX::SimpleMath::Vector3& pos,
		const DirectX::SimpleMath::Vector3& dir,
		std::shared_ptr<DirectX::Model> model,
		DirectX::CommonStates* states,
		std::shared_ptr<DisplayCollision> displayCollision
	);

	//更新
	void Update(float dt) override;

	//描画
	void Render(
		ID3D11DeviceContext* context,
		const DirectX::SimpleMath::Matrix& view,
		const DirectX::SimpleMath::Matrix& proj)override;

	//やられた？
	bool IsDead()const override { return m_dead; }

	virtual void SetDead() override { m_dead = true; }

	//ノックバックの大きさ
	virtual float GetKnockbackPower() const override { return 10.0f; }

	virtual ModelCollision* GetCollision() const override { return m_collision.get(); }

	DirectX::SimpleMath::Vector3 GetPosition() const override { return m_pos; }

	//ダメージ量
	float GetDamage() const override { return 20.0f; }

private:

	DirectX::SimpleMath::Vector3 m_pos;
	DirectX::SimpleMath::Vector3 m_vel;

	DirectX::SimpleMath::Vector3 m_forward;

	bool m_dead = false;

	

	std::shared_ptr<DirectX::Model> m_model;

	DirectX::CommonStates* m_states;
	//std::unique_ptr<DirectX::CommonStates> m_states;


	std::unique_ptr<ModelCollisionOrientedBox> m_collision;
	std::shared_ptr<DisplayCollision> m_displayCollision;
};

