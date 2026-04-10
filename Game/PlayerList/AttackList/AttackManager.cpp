#include "pch.h"
#include "Game/PlayerList/AttackList/AttackManager.h"
#include "Game/PlayerList/AttackList/BulletP.h"
#include "Game/PlayerList/Player.h"
#include "Game/PlayerList/AttackList/AttackP.h"
#include "Game/PlayerList/AttackList/RushP.h"

#include "Game/EnemyList/BossEnemy.h"

//効果音
#include "Game/SoundList/AudioManager.h"


//-----------------------------------------------------------------
// 遠距離攻撃（弾）を生成して発射
//-----------------------------------------------------------------

void AttackManager::Bullet(Player* player)
{
	using namespace DirectX::SimpleMath;

	if (!m_bulletModel)
	{
		return;
	}

	//カメラ視線方向を取得
	Vector3 eye = player->GetCamera()->GetEyePosition();
	Vector3 target = player->GetCamera()->GetTargetPosition();
	Vector3 dir = player->GetForward();
	//dir.y = 0;
	dir.Normalize();

	//弾の生成位置
	Vector3 spawnPos =
		player->GetPosition() 
		+ dir * SPAWN_FORWARD_DIST
		+ Vector3(0, SPAWN_HEIGHT_OFFSET, 0);

	auto bullet = std::make_shared<BulletP>(
		spawnPos,
		dir,
		m_bulletModel,
		m_states,
		player->GetStage(),
		player->GetDisplayCollision()
	);

	AddAttack(bullet);

	//効果音
	AudioManager::GetInstance()->Play("Bullet");
}

//-----------------------------------------------------------------
// 近距離攻撃を生成して実行
//-----------------------------------------------------------------

void AttackManager::Attack(Player* player)
{
	if (!player || !player->GetDisplayCollision())
		return;

	// 近距離攻撃の生成
	auto meleeAttack = std::make_shared<AttackP>(
		player->GetDeviceResources(),   // PlayerからDeviceResources取得
		player->GetPosition(),          // プレイヤー位置
		player->GetForward(),           // プレイヤー前方
		player->GetDisplayCollision()   // 当たり判定表示用
	);

	AddAttack(meleeAttack);

}

//-----------------------------------------------------------------
// 突進攻撃を生成して実行
//-----------------------------------------------------------------

void AttackManager::Rush(Player* player)
{
	using namespace DirectX::SimpleMath;

	//プレイヤーの位置と前方ベクトルを取得
	Vector3 playerPos = player->GetPosition();
	Vector3 dir = player->GetForward();

	//RushPのインスタンス化
	auto rushAttack = std::make_shared<RushP>(
		player->GetDeviceResources(), // DeviceResources
		player,                     // playerPos
		dir,                           // forward
		m_displayCollision  // displayCollision
	);

	AddAttack(rushAttack);

}

//-----------------------------------------------------------------
// 生成された遠距離攻撃（弾）の攻撃オブジェクトを管理リストに追加
//-----------------------------------------------------------------

void AttackManager::AddAttack(std::shared_ptr<AttackBase> attack)
{
	m_attacks.push_back(attack);
}

//-----------------------------------------------------------------
// 攻撃の更新処理と敵の当たり判定
//-----------------------------------------------------------------

void AttackManager::Update(
	float dt,
	std::vector<BossEnemy*>& enemies,
	Particle* particle)
{
	for (auto& atk : m_attacks)
	{
		atk->Update(dt);
	}

	//ノックバック
	for (auto& atk : m_attacks)
	{
		//既にヒットして無効化されている攻撃や、時間切れの攻撃はスキップ
		if (atk->IsDead()) continue;

		//敵との衝突チェック
		for (auto& enemy : enemies)
		{
			if (enemy->IsDead()) continue;

			//安全のためのnullptrチェック
			ModelCollision* atkCol = atk->GetCollision();
			ModelCollision* enemyCol = enemy->GetCollision();

			if (atkCol == nullptr || enemyCol == nullptr) {
				//衝突情報がないオブジェクトは衝突判定を行わずスキップ
				continue;
			}

			//衝突判定が成功した場合
			if (atkCol->Intersects(enemyCol))
			{
				//ノックバック方向の計算 (攻撃源 -> ターゲット)
				DirectX::SimpleMath::Vector3 attackPos = atk->GetPosition();
				DirectX::SimpleMath::Vector3 targetPos = enemy->GetPosition();

				//ターゲットを攻撃源から押し返すベクトル
				DirectX::SimpleMath::Vector3 knockbackDirection = atk->GetForward();
				knockbackDirection.Normalize();

				float basePower = atk->GetKnockbackPower();

				//エフェクト
				if (particle)
				{
					//敵の座標を取得
					DirectX::SimpleMath::Vector3 hitPos = enemy->GetPosition();
					//高さ上げる
					hitPos.y += 1.0f;
					//パーティクル生成
					particle->Spawn(Particle::Type::Explosion, hitPos, 10, 0.1);

				}

				//ノックバック
				enemy->ApplyKnockback(knockbackDirection, basePower);
				
				//攻撃の種類を判別する変数
				BossEnemy::PlayerAttackType type = BossEnemy::PlayerAttackType::None;
				//ダメージを調整用 個別で変更可
				float damage = 100.0f;

				// 弾（BulletP）だった場合
				if (dynamic_cast<BulletP*>(atk.get()))
				{
					//種類
					type = BossEnemy::PlayerAttackType::Shoot;

					//音
					AudioManager::GetInstance()->Play("Dash");
				}
				// 近接攻撃（AttackP）だった場合
				else if (dynamic_cast<AttackP*>(atk.get()))
				{
					//種類
					type = BossEnemy::PlayerAttackType::Attack;

					//音
					AudioManager::GetInstance()->Play("Attack");
				}
				// ラッシュ（RushP）だった場合
				else if (RushP* rushAtk = dynamic_cast<RushP*>(atk.get()))
				{
					//種類
					type = BossEnemy::PlayerAttackType::Rush;

					//音
					AudioManager::GetInstance()->Play("Dash");

					//敵に攻撃が当たったらプレイヤーにも与える
					Player* hitPlayer = rushAtk->GetPlayer();

					if (hitPlayer != nullptr)
					{
						//プレイヤーの向いている方向とは逆のベクトルを計算
						DirectX::SimpleMath::Vector3 bounceDir = -hitPlayer->GetForward();

						//プレイヤーにノックバックを与える
						hitPlayer->ApplyKnockback(bounceDir, 2.0f);
					}
				}


				//判別した種類で攻撃
				enemy->TakeDamage(damage, type);

				//攻撃を消す
				atk->SetDead();
			}
		}
	}

	//弾を削除
	m_attacks.erase(
		std::remove_if(
			m_attacks.begin(),
			m_attacks.end(),
			[](const std::shared_ptr<AttackBase>& a)
			{
				return a->IsDead();
			}
		),
		m_attacks.end()
	);
}

//-----------------------------------------------------------------
// 攻撃のモデルを描画
//-----------------------------------------------------------------

void AttackManager::Render(
	ID3D11DeviceContext* context,
	const DirectX::SimpleMath::Matrix& view,
	const DirectX::SimpleMath::Matrix& proj)
{
	for (auto& atk : m_attacks)
	{
		atk->Render(context, view, proj);
	}
}

//-----------------------------------------------------------------
// 弾の描画に使うモデルの設定
//-----------------------------------------------------------------

void AttackManager::SetBulletModel(std::shared_ptr<DirectX::Model> model)
{
	m_bulletModel = model;
}

//-----------------------------------------------------------------
// 描画ステートを設定
//-----------------------------------------------------------------

void AttackManager::SetStates(DirectX::CommonStates* states)
{
	m_states = states;
}
