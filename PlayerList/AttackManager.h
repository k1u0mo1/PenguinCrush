
//’eiUŒ‚j‚ğ‚Ü‚Æ‚ß‚ÄŠÇ—‚·‚é

#pragma once

#include "AttackBase.h"
#include <memory>
#include <vector>
#include <Model.h>

#include "Collision/DisplayCollision.h"


class BossEnemy;
class Player;
class Particle;

class AttackManager
{
public: 

	AttackManager() = default;
	~AttackManager() = default;

	// ‰“‹——£UŒ‚
	void Bullet(Player* player);  

	// ‹ß‹——£UŒ‚
	void Attack(Player* player); 

	// ƒ‰ƒbƒVƒ…UŒ‚
	void Rush(Player* player);   

	//’e‚ğ’Ç‰Á
	void AddAttack(std::shared_ptr<AttackBase> attack);

	//XV
	void Update(float dt, std::vector<BossEnemy*>& enemies,Particle* particle);

	//•`‰æ
	void Render(
		ID3D11DeviceContext* context,
		const DirectX::SimpleMath::Matrix& view,
		const DirectX::SimpleMath::Matrix& proj
	);

	//’e‚Ìƒ‚ƒfƒ‹‚ğİ’è
	void SetBulletModel(std::shared_ptr<DirectX::Model> model);

	void SetStates(DirectX::CommonStates* states);

	void SetDisplayCollision(std::shared_ptr<DisplayCollision>collision)
	{
		m_displayCollision = collision;
	}

private:

	std::vector<std::shared_ptr<AttackBase>> m_attacks;

	std::shared_ptr<DirectX::Model> m_bulletModel;

	DirectX::CommonStates* m_states = nullptr;

	//ƒRƒŠƒWƒ‡ƒ“Á‚·
	std::shared_ptr<DisplayCollision> m_displayCollision;

};
