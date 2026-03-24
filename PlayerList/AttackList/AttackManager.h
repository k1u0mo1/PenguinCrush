
//弾（攻撃）をまとめて管理する

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
private:

	//------------------------------------------------------
	//定数関連
	//------------------------------------------------------

	//弾の速度
	static constexpr float SPAWN_FORWARD_DIST = 1.5f;
	//弾の生成する高さ
	static constexpr float SPAWN_HEIGHT_OFFSET = 1.5f;

public: 
	
	AttackManager() = default;
	~AttackManager() = default;

	/// <summary>
	/// 遠距離攻撃（弾）を生成して発射
	/// </summary>
	/// <param name="player">攻撃を行うプレイヤーのポインタ</param>
	void Bullet(Player* player);  

	/// <summary>
	/// 近距離攻撃を生成して実行
	/// </summary>
	/// <param name="player">攻撃を行うプレイヤーのポインタ</param>
	void Attack(Player* player); 

	/// <summary>
	/// 突進攻撃を生成して実行
	/// </summary>
	/// <param name="player">攻撃を行うプレイヤーのポインタ</param>
	void Rush(Player* player);   

	/// <summary>
	/// 生成された遠距離攻撃（弾）の攻撃オブジェクトを管理リストに追加
	/// </summary>
	/// <param name="attack">追加する攻撃の共有ポインタ</param>
	void AddAttack(std::shared_ptr<AttackBase> attack);

	/// <summary>
	/// 攻撃の更新処理と敵の当たり判定
	/// </summary>
	/// <param name="dt">前フレームからの経過時間</param>
	/// <param name="enemies">ステージ上に存在するボスのリスト</param>
	/// <param name="particle">ヒット時のエフェクト発生用マネージャー</param>
	void Update(float dt, std::vector<BossEnemy*>& enemies,Particle* particle);

	/// <summary>
	/// 攻撃のモデルを描画
	/// </summary>
	/// <param name="context">デバイスコンテキスト</param>
	/// <param name="view">ビュー行列</param>
	/// <param name="proj">射影行列</param>
	void Render(
		ID3D11DeviceContext* context,
		const DirectX::SimpleMath::Matrix& view,
		const DirectX::SimpleMath::Matrix& proj
	);

	/// <summary>
	/// 弾の描画に使うモデルの設定
	/// </summary>
	/// <param name="model"></param>
	void SetBulletModel(std::shared_ptr<DirectX::Model> model);

	/// <summary>
	/// 描画ステートを設定
	/// </summary>
	/// <param name="states"></param>
	void SetStates(DirectX::CommonStates* states);

	/// <summary>
	/// 表示用のコリジョンを設定
	/// </summary>
	/// <param name="collision"></param>
	void SetDisplayCollision(std::shared_ptr<DisplayCollision>collision)
	{
		m_displayCollision = collision;
	}

private:

	std::vector<std::shared_ptr<AttackBase>> m_attacks;

	std::shared_ptr<DirectX::Model> m_bulletModel;

	DirectX::CommonStates* m_states = nullptr;

	//コリジョン消す
	std::shared_ptr<DisplayCollision> m_displayCollision;

};
