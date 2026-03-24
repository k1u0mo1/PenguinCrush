
//攻撃の基底クラス-------------------------------

#pragma once
#include <DirectXMath.h>
#include <memory>
#include <SimpleMath.h>
#include <d3d11.h>

class ModelCollision;

/// <summary>
/// プレイヤーの攻撃のベースとなる純粋仮想クラス
/// </summary>
class AttackBase
{
public: 

	virtual ~AttackBase() = default;

	/// <summary>
	/// 攻撃の更新処理
	/// </summary>
	/// <param name="dt">前フレームからの経過時間</param>
	virtual void Update(float dt) = 0;

	/// <summary>
	/// 攻撃のモデルやエフェクトを描画
	/// </summary>
	/// <param name="context"></param>
	/// <param name="view"></param>
	/// <param name="proj"></param>
	virtual void Render(
		ID3D11DeviceContext* context,
		const DirectX::SimpleMath::Matrix& view,
		const DirectX::SimpleMath::Matrix& proj
	) = 0;

	/// <summary>
	/// 攻撃が終了したか判断
	/// </summary>
	/// <returns>終了している場合はtrue、まだ有効な場合はfalseを返す</returns>
	virtual bool IsDead() const = 0;

	/// <summary>
	/// 攻撃を強制的に無効化
	/// </summary>
	virtual void SetDead() = 0;

	/// <summary>
	/// 敵にヒットした際に与えるノックバックの強さを取得
	/// </summary>
	/// <returns>ノックバック力</returns>
	virtual float GetKnockbackPower() const = 0;

	
	/// <summary>
	/// 攻撃の当たり判定オブジェクトを取得
	/// </summary>
	/// <returns>当たり判定のポインタ</returns>
	virtual ModelCollision* GetCollision() const = 0;

	/// <summary>
	/// 現在のワールド座標を取得
	/// </summary>
	/// <returns></returns>
	virtual DirectX::SimpleMath::Vector3 GetPosition() const = 0;

	/// <summary>
	/// 現在の進行方向を取得
	/// </summary>
	/// <returns></returns>
	DirectX::SimpleMath::Vector3 GetForward()const { return m_forward; }


	/// <summary>
	/// 攻撃の基本ダメージ量を取得
	/// </summary>
	/// <returns></returns>
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
