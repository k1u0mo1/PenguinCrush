

/**
 * @file   EnemyBase.h
 * @brief  敵の管理を行う基底クラス
 * @author 國田知睦
 * @date   2026/07/01
 */

#pragma once
#include "pch.h"
#include <Game/Common/DeviceResources.h>
#include "Game/Common/ObjectCharacter/CharacterBase.h"
#include "Game/EnemyList/EnemyBaseParameter.h"
#include "Game/EnemyList/EnemyAI.h"

class Stage;
class Particle;
class EnemyManager;

/// <summary>
/// 敵の状態を表す列挙型
/// </summary>
enum class EnemyState
{
	Opening,
	Loading,
	Idle,
	Attack,
	Rush,
	Avoid,
	Dizzy,
	Down
};

enum class PlayerAttackType
{
	None,
	Attack, // 近接
	Rush    // 突進
};

/// <summary>
/// 敵の基底クラス
/// </summary>
class EnemyBase : public CharacterBase
{
protected:

	//落下の判定の高さ
	static constexpr float FALL_LIMIT_Y = -5.0f;
	//接地する足場の位置の高さ
	static constexpr float STAGE_BOUNDARY_Y = -4.9f;
	//落下ダメージ
	static constexpr float FALL_DAMAGE = 200.0f;
	//復活する高さ
	static constexpr float RESPAWN_HEIGHT = 10.0f;

	//水しぶきの粒子数
	static constexpr int SPLASH_PARTICLE_COUNT = 20;

	//ステージの範囲
	static constexpr float CHECK_RANDE = 10.0f;
	//確認する間隔の間
	static constexpr float CHECK_INTERVAL = 2.0f;

public:

	/// <summary>
	/// 敵のインスタンスを生成
	/// </summary>
	/// <param name="deviceResources">デバイス</param>
	/// <param name="position">初期位置</param>
	/// <param name="param">敵のパラメータ</param>
	EnemyBase(DX::DeviceResources* deviceResources,
		const DirectX::SimpleMath::Vector3& position,
		const EnemyBaseParameter& param);

	/// <summary>
	/// 敵の初期化
	/// </summary>
	virtual void Initialize();

	/// <summary>
	/// 敵の更新
	/// </summary>
	/// </summary>
	/// <param name="dt">経過時間</param>
	/// <param name="playerPos">プレイヤーの位置</param>
	/// <param name="stage">ステージ情報</param>
	/// <param name="particle">パーティクル情報</param>
	/// <param name="enemyManager">敵管理情報</param>
	virtual void Update(
		float dt,
		const DirectX::SimpleMath::Vector3& playerPos,
		Stage* stage,
		Particle* particle,
		EnemyManager* enemyManager
	);

	/// <summary>
	/// 敵の描画
	/// </summary>
	/// <param name="context">デバイスコンテキスト</param>
	/// <param name="view">ビュー行列</param>
	/// <param name="proj">射影行列</param>
	virtual void Render(
		ID3D11DeviceContext* context,
		const DirectX::SimpleMath::Matrix& view,
		const DirectX::SimpleMath::Matrix& proj
	) = 0;

	/// <summary>
	/// 敵が倒されたかどうか
	/// </summary>
	/// <returns>敵のHPが0以下なら true</returns>
	virtual bool IsDead() const  { return m_hp <= 0.0f; }

	/// <summary>
	/// 敵の位置を強制的に設定
	/// </summary>
	/// <param name="position">新しい位置</param>
	void SetPosition(
		const DirectX::SimpleMath::Vector3& position) { m_position = position; }

	/// <summary>
	/// 敵の体の向きを設定
	/// </summary>
	/// <param name="rotationY">Y軸回転角度（ラジアン）</param>
	void SetRotationY(float rotationY) { m_rotationY = rotationY; }

	/// <summary>
	/// モデルの当たり判定を取得
	/// </summary>
	/// <returns>モデルの当たり判定のポインタ</returns>
	virtual ModelCollision* GetCollision() { return nullptr; }
	
	/// <summary>
	/// 敵のHPを減らす処理
	/// </summary>
	/// <param name="amount">減少させるHPの量</param>
	/// <param name="type">攻撃の種類</param>
	virtual void TakeDamage(float /*amount*/, PlayerAttackType /*type*/){}

	/// <summary>
	/// 敵のノックバックを適用する処理
	/// </summary>
	/// <param name="direction">ノックバックの方向</param>
	/// <param name="power">ノックバックの強さ</param>
	virtual void ApplyKnockback(const DirectX::SimpleMath::Vector3& /*direction*/,float /*power*/){}

	/// <summary>
	/// 敵の落下の判定と処理
	/// </summary>
	/// <param name="stage">ステージ情報</param>
	/// <param name="particle">パーティクル情報</param>
	void CheckAndHandleFalling(Stage* stage, Particle* particle);

	/// <summary>
	/// 敵クラスに情報を渡すよう
	/// </summary>
	/// <returns>敵のパラメータ</returns>
	const EnemyBaseParameter& GetParam() const { return m_param; }

protected:

	/// <summary>
	/// 敵のAIの思考処理
	/// </summary>
	/// <param name="dt">経過時間</param>
	/// <param name="playerPos">プレイヤーの位置</param>
	/// <param name="stage">ステージ情報</param>
	/// <param name="particle">パーティクル情報</param>
	/// <param name="enemyManager">敵管理情報</param>
	virtual void UpdateAI(
		float dt,
		const DirectX::SimpleMath::Vector3& playerPos,
		Stage* stage,
		Particle* particle,
		EnemyManager* enemyManager
	) = 0;

protected:

	//デバイスリソース
	DX::DeviceResources* m_deviceResources;
	//敵のパラメータ
	EnemyBaseParameter m_param;
	
	float m_rotationY;
	
	std::unique_ptr<EnemyAI> m_enemyAI;
};