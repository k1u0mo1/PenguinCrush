
/**
 * @file   NormalEnemy.h
 * @brief  通常敵の管理を行うクラス
 * @author 國田知睦
 * @date   2026/06/24
 */

#pragma once
#include "pch.h"

#include "EnemyBase.h"
#include "Game/Common/DeviceResources.h"
#include "Game/Collision/DisplayCollision.h"
#include "Game/Collision/ModelCollision.h"
#include "Game/GimmickList/SlideBehavior.h"
#include "Game/EnemyList/EnemyRenderer.h"
#include "Game/EnemyList/BossEnemy.h"
#include "Game/EnemyList/EnemyBaseParameter.h"

class NormalEnemy : public EnemyBase
{
private:
	//落下の判定の高さ
	static constexpr float FALL_LIMIT_Y = -5.0f;
	//接地する足場の位置の高さ
	static constexpr float STAGE_BOUNDARY_Y = -4.9f;
	//落下時に受けるダメージ量
	static constexpr float FALL_DAMAGE = 10.0f;
	//プレイヤーとの距離（近すぎないように）
	static constexpr float STOP_DISTANCE = 5.0f;
	
	//敵の速度
	static constexpr float ENEMY_SPEED = 0.9f;
	//個体差用の速度のランダムな振れ幅
	static constexpr float SPEED_RAND_RANGE = 0.15f;
	//プレイヤーとの停止するランダムな振れ幅
	static constexpr float STOP_DISTANCE_RAND_RANGE = 1.5f;

	//水しぶきの粒子数
	static constexpr float SPLASH_PARTICLE_COUNT = 20.0f;

	//攻撃する距離
	static constexpr float ATTACK_DISTANCE = 2.0f;
	//復活する時の判定の高さ
	static constexpr float RESPAWN_HEIGHT = 10.0f;

	//攻撃ステートのタイマー
	static constexpr float ATTACK_STATE_DURATION = 1.0f;
	//クールダウン
	static constexpr float ATTACK_COOLDOWN = 1.5f;

	//コリジョンの透明度
	static constexpr float COLLISION_DRAW_ALPHA_NORMAL = 0.15f;
	//モデルコリジョンの透明度
	static constexpr float COLLISION_DRAW_ALPHA_ACTIVE = 0.5f;

	//微小値
	static constexpr float VECTOR_EPSILON = 0.0001f;

public:

	/// <summary>
	/// 敵のインスタンスを生成
	/// </summary>
	/// <param name="deviceResources">デバイス</param>
	/// <param name="position">初期位置</param>
	/// <param name="param">敵のパラメータ</param>
	/// <param name="displayCollision">コリジョンの表示情報</param>
	NormalEnemy(DX::DeviceResources* deviceResources,
		const DirectX::SimpleMath::Vector3& position,
		const EnemyBaseParameter& param,
		std::shared_ptr<DisplayCollision> displayCollision);

	/// <summary>
	/// 敵の初期化
	/// </summary>
	void Initialize() override;

	/// <summary>
	/// 通常の敵の更新
	/// </summary>
	/// <param name="deltaTime">経過時間</param>
	/// <param name="playerPosition">プレイヤーの位置</param>
	/// <param name="stage">ステージ情報</param>
	/// <param name="particle">パーティクル情報</param>
	/// <param name="enemyManager">敵管理情報</param>
	void Update(
		float deltaTime,
		const DirectX::SimpleMath::Vector3& playerPosition,
		Stage* stage,
		Particle* particle,
		EnemyManager* enemyManager=nullptr
	);

	/// <summary>
	/// 敵の描画
	/// </summary>
	/// <param name="context">デバイスコンテキスト</param>
	/// <param name="view">ビュー行列</param>
	/// <param name="proj">射影行列</param>
	void Render(
		ID3D11DeviceContext* context,
		const DirectX::SimpleMath::Matrix& view,
		const DirectX::SimpleMath::Matrix& proj
	) override;

	/// <summary>
	/// 通常の敵の当たり判定を渡す
	/// </summary>
	/// <returns></returns>
	ModelCollision* GetCollision() override { return m_modelCollision.get(); }

	/// <summary>
	/// ダメージを受けたときの処理
	/// </summary>
	/// <param name="amount">ダメージ量</param>
	/// <param name="type">プレイヤーから受けた攻撃の種類</param>
	void TakeDamage(float amount, PlayerAttackType type);

	/// <summary>
	/// 攻撃を受けた際のノックバック力を適用
	/// </summary>
	/// <param name="direction">吹き飛ぶ方向ベクトル</param>
	/// <param name="power">ノックバックの強さ</param>
	void ApplyKnockback(const DirectX::SimpleMath::Vector3& direction, float power)override;

protected:

	/// <summary>
	/// 敵の更新
	/// </summary>
	/// </summary>
	/// <param name="dt">経過時間</param>
	/// <param name="playerPos">プレイヤーの位置</param>
	/// <param name="stage">ステージ情報</param>
	/// <param name="particle">パーティクル情報</param>
	/// <param name="enemyManager">敵管理情報</param>
	void UpdateAI(
		float dt,
		const DirectX::SimpleMath::Vector3& playerPos,
		Stage* stage,
		Particle* particle,
		EnemyManager* enemyManager
	) override;

	/// <summary>
	/// 通常敵の物理更新
	/// </summary>
	/// <param name="stage">ステージ情報</param>
	/// <param name="dt">経過時間</param>
	/// <param name="particle">パーティクル情報</param>
	void UpdatePhysics(Stage* stage, float dt, Particle* particle = nullptr);

protected:

	/// <summary>
	/// 敵の落下判定の高さを取得
	/// </summary>
	/// <returns>落下判定の高さ</returns>
	float GetFallLimitY() const override { return FALL_LIMIT_Y; }

	/// <summary>
	/// 敵の落下ダメージを取得
	/// </summary>
	/// <returns>落下ダメージの量</returns>
	float GetFallDamage() const override { return FALL_DAMAGE; }

	/// <summary>
	/// 敵が落下したときの水しぶきの粒子数を取得
	/// </summary>
	/// <returns>水しぶきの粒子数</returns>
	int GetSplashParticleCount() const override { return static_cast<int>(SPLASH_PARTICLE_COUNT); } 
	
	/// <summary>
	/// 敵のリスポーン位置を取得
	/// </summary>
	/// <returns></returns>
	DirectX::SimpleMath::Vector3 GetRespawnPosition() const override 
	{
		return DirectX::SimpleMath::Vector3(0.0f, RESPAWN_HEIGHT, 0.0f); 
	}

	/// <summary>
	/// 敵が落下ダメージを受けるときの処理
	/// </summary>
	void ExecuteFallDamage() override 
	{

		TakeDamage(GetFallDamage(), PlayerAttackType::Attack);
	}

private:

	//デバイスリソース
	DX::DeviceResources* m_deviceResources;
	//当たり判定の表示用のポインタ
	std::shared_ptr<DisplayCollision> m_displayCollision;
	//当たり判定のモデル用のポインタ
	std::unique_ptr<ModelCollision> m_modelCollision;
	//描画用のステート
	std::unique_ptr<DirectX::CommonStates> m_states;
	//敵のモデルやテクスチャなどの描画に必要なリソースを管理するクラス
	std::unique_ptr<EnemyRenderer> m_renderer;

	//滑る床の挙動を管理するクラス
	SlideBehavior m_slideBehavior;

	//敵の自発的な移動に対する慣性
	DirectX::SimpleMath::Vector3 m_targetVelocity = DirectX::SimpleMath::Vector3::Zero;
	float m_rotationY = 0.0f;
	DirectX::SimpleMath::Vector3 m_forward = { 0.0f, 0.0f, 0.0f };

	//
	std::vector<std::shared_ptr<EnemyAttackPattern>> m_attackPattern;
	float m_meleeAttackCooldownTimer = 0.0f;

	//敵の状態
	EnemyState m_state = EnemyState::Idle;
	float m_stateTimer = 0.0f;

	//敵の個体差によってプレイヤーとの距離を変える
	float m_stopDistance;

};