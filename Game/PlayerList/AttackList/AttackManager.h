
/**
 * @file   AttackManager.h
 * @brief  攻撃の管理クラス
 * @author 國田知睦
 * @date   2026/07/01
 */

#pragma once
#include "AttackBase.h"
#include <memory>
#include <vector>
#include <Model.h>
#include "Game/Collision/DisplayCollision.h"

class EnemyBase;
class Player;
class Particle;
class HitStop;

class AttackManager
{
private:

	//エフェクトの高さ
	static constexpr float HIT_EFFECT_HEIGHT = 1.0f;
	//エフェクトの生成数
	static constexpr int HIT_EFFECT_COUNT = 50;
	//エフェクトのサイズ
	static constexpr float HIT_EFFECT_SIZE = 0.1f;
	
	//ダメージ量
	static constexpr float ATTACK_DAMAGE = 100.0f;
	//ダッシュのノックバックの強さ
	static constexpr float RUSH_KNOCKBACK_POWER = 10.0f;
	//ヒットストップの時間
	static constexpr float HIT_STOP_DURATION = 0.1f;

public: 
	
	AttackManager() = default;
	~AttackManager() = default;

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
	/// <param name="enemies">ステージ上に存在する敵のリスト</param>
	/// <param name="particle">ヒット時のエフェクト発生用マネージャー</param>
	void Update(float dt, std::vector<EnemyBase*>& enemies,Particle* particle,HitStop* hitStop);

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
	/// 描画ステートを設定
	/// </summary>
	/// <param name="states">描画に使用する共通ステート</param>
	void SetStates(DirectX::CommonStates* states);

	/// <summary>
	/// 表示用のコリジョンを設定
	/// </summary>
	/// <param name="collision">当たり判定コリジョン</param>
	void SetDisplayCollision(std::shared_ptr<DisplayCollision>collision)
	{
		m_displayCollision = collision;
	}

private:
	//攻撃リスト
	std::vector<std::shared_ptr<AttackBase>> m_attacks;

	DirectX::CommonStates* m_states = nullptr;

	//コリジョン消す
	std::shared_ptr<DisplayCollision> m_displayCollision;
};
