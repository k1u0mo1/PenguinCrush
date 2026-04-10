
//ModelCollision.h
//モデルの衝突判定

#pragma once
#include "Game/Collision/DisplayCollision.h"
#include <Model.h>

class ModelCollisionSphere;
class ModelCollisionBox;
class ModelCollisionOrientedBox;

//衝突判定の基底クラス
class ModelCollision
{
public:

	//コリジョンのタイプ
	enum class CollisionType
	{
		None,
		Sphere,
		AABB,
		OBB,
	};

	/// <summary>
	/// 別の衝突判定オブジェクトと衝突しているか判定
	/// </summary>
	/// <param name="other">判定対象のモデルコリジョン</param>
	/// <returns>衝突していればtrue、していなければfalse</returns>
	bool Intersects(const ModelCollision& other) const;

	/// <summary>
	/// 衝突判定全体を囲むAABBの最小座標を取得
	/// </summary>
	/// <returns>AABBの最小座標</returns>
	DirectX::SimpleMath::Vector3 GetAABBMin() const;

	/// <summary>
	/// 衝突判定全体を囲むAABBの最大座標を取得
	/// </summary>
	/// <returns>AABBの最大座標</returns>
	DirectX::SimpleMath::Vector3 GetAABBMax() const;

	//ModelCollision(const DirectX::SimpleMath::Vector3& size); // size = extents * 2

	/// <summary>
	/// 衝突判定の中心座標を取得
	/// </summary>
	/// <returns>中心座標を示すベクトルへの参照</returns>
	const DirectX::SimpleMath::Vector3& GetCenter() const;

	/// <summary>
	/// 衝突判定の各軸方向のサイズ（Extents / 半幅）を取得
	/// </summary>
	/// <returns>サイズを示すベクトルへの参照</returns>
	const DirectX::SimpleMath::Vector3& GetExtents() const;

private:

	//コリジョンタイプ
	CollisionType m_type;

	//コリジョン全体を囲む方向付き境界ボックス　OBB
	DirectX::BoundingOrientedBox m_boundingOrientedBox;

public:

	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="type">生成するコリジョンの形状タイプ</param>
	ModelCollision(CollisionType type);

	/// <summary>
	/// デストラクタ
	/// </summary>
	virtual ~ModelCollision() {}

	/// <summary>
	/// 衝突判定を行う関数
	/// </summary>
	/// <param name="collision">判定対象の相手コリジョンへのポインタ</param>
	/// <returns>衝突していればtrue、していなければfalse</returns>
	virtual bool Intersects(ModelCollision* collision) = 0;

	/// <summary>
	/// 内包判定を行う関数
	/// </summary>
	/// <param name="collision">判定対象の相手コリジョンへのポインタ</param>
	/// <returns>完全に内包していればtrue、していなければfalse</returns>
	virtual bool Contains(ModelCollision* collision) = 0;

	/// <summary>
	/// コリジョンタイプの取得関数
	/// </summary>
	/// <returns>コリジョンタイプ</returns>
	CollisionType GetType() const { return m_type; }

	/// <summary>
	/// コリジョン情報の更新
	/// </summary>
	/// <param name="position">モデルの現在のワールド座標</param>
	/// <param name="rotate">モデルの現在の回転姿勢</param>
	/// <param name="scale">モデルの現在のスケール値</param>
	virtual void UpdateBoundingInfo(
		DirectX::SimpleMath::Vector3 position,
		DirectX::SimpleMath::Quaternion rotate,
		DirectX::SimpleMath::Vector3 scale =
		DirectX::SimpleMath::Vector3(1.0f, 1.0f, 1.0f)) = 0;

	/// <summary>
	/// 衝突判定の表示に登録する関数
	/// </summary>
	/// <param name="displayCollision">登録先となるデバッグ表示用クラスのポインタ</param>
	virtual void AddDisplayCollision(DisplayCollision* displayCollision) = 0;

	/// <summary>
	/// 境界情報
	/// </summary>
	/// <param name="world">モデルに適用されるワールド変換行列</param>
	virtual void UpdateBoundingInfo(const DirectX::SimpleMath::Matrix& world) = 0;

protected:

	/// <summary>
	/// 衝突判定（複数メッシュに対応）
	/// </summary>
	/// <typeparam name="T">自身の境界データのコレクション型</typeparam>
	/// <typeparam name="U">相手の境界データのコレクション型</typeparam>
	/// <param name="a">>自身の境界データリスト</param>
	/// <param name="b">相手の境界データリスト</param>
	/// <returns>1つでも衝突していればtrue、一切衝突していなければfalse</returns>
	template <class T, class U>
	bool Intersects(T a, U b)
	{
		for (auto& data_a : a)
		{
			for (auto& data_b : b)
			{
				if (data_a.Intersects(data_b))
				{
					return true;
				}
			}
		}
		return false;
	}

	/// <summary>
	/// 衝突判定
	/// </summary>
	/// <typeparam name="T">自身の境界データのコレクション型</typeparam>
	/// <typeparam name="U">ダウンキャスト後の相手の境界データ型</typeparam>
	/// <param name="a">自身の境界データリスト</param>
	/// <param name="b">判定相手のModelCollision基底ポインタ</param>
	/// <returns>衝突していればtrue、していなければfalse</returns>
	template <class T, class U>
	bool Intersects(T* a, ModelCollision* b)
	{
		if (b == nullptr) return false;

		switch (b->GetType())
		{
		case CollisionType::Sphere:
		{
			auto p = dynamic_cast<ModelCollisionSphere*>(b);
			assert(p);
			return ModelCollision::Intersects<const std::vector<U>&, const std::vector<DirectX::BoundingSphere>&>
				(a->GetBoundingInfo(), p->GetBoundingInfo());
		}
		case CollisionType::AABB:
		{
			auto p = dynamic_cast<ModelCollisionBox*>(b);
			assert(p);
			return ModelCollision::Intersects<const std::vector<U>&, const std::vector<DirectX::BoundingBox>&>
				(a->GetBoundingInfo(), p->GetBoundingInfo());
		}
		case CollisionType::OBB:
		{
			auto p = dynamic_cast<ModelCollisionOrientedBox*>(b);
			assert(p);
			return ModelCollision::Intersects<const std::vector<U>&, const std::vector<DirectX::BoundingOrientedBox>&>
				(a->GetBoundingInfo(), p->GetBoundingInfo());
		}
		default:
			break;
		}
		return false;

	}

	/// <summary>
	/// 内包判定
	/// </summary>
	/// <typeparam name="T">自身の境界データのコレクション型</typeparam>
	/// <typeparam name="U">相手の境界データのコレクション型</typeparam>
	/// <param name="a">自身の境界データリスト</param>
	/// <param name="b">相手の境界データリスト</param>
	/// <returns>完全に内包していればtrue、していなければfalse</returns>
	template <class T, class U>
	bool Contains(T a, U b)
	{
		for (auto& data_a : a)
		{
			for (auto& data_b : b)
			{
				if (data_a.Contains(data_b) != DirectX::CONTAINS)
				{
					return false;
				}
			}
		}
		return true;
	}

	/// <summary>
	/// 内包判定
	/// </summary>
	/// <typeparam name="T">自身の境界データのコレクション型</typeparam>
	/// <typeparam name="U">ダウンキャスト後の相手の境界データ型</typeparam>
	/// <param name="a">自身の境界データリスト</param>
	/// <param name="b">判定相手のModelCollision基底ポインタ</param>
	/// <returns>完全に内包していればtrue、していなければfalse</returns>
	template <class T, class U>
	bool Contains(T* a, ModelCollision* b)
	{
		if (b == nullptr) return false;

		switch (b->GetType())
		{
		case CollisionType::Sphere:
		{
			auto p = dynamic_cast<ModelCollisionSphere*>(b);
			assert(p);
			return ModelCollision::Contains<const std::vector<U>&, const std::vector<DirectX::BoundingSphere>&>
				(a->GetBoundingInfo(), p->GetBoundingInfo());
		}
		case CollisionType::AABB:
		{
			auto p = dynamic_cast<ModelCollisionBox*>(b);
			assert(p);
			return ModelCollision::Contains<const std::vector<U>&, const std::vector<DirectX::BoundingBox>&>
				(a->GetBoundingInfo(), p->GetBoundingInfo());
		}
		case CollisionType::OBB:
		{
			auto p = dynamic_cast<ModelCollisionOrientedBox*>(b);
			assert(p);
			return ModelCollision::Contains<const std::vector<U>&, const std::vector<DirectX::BoundingOrientedBox>&>
				(a->GetBoundingInfo(), p->GetBoundingInfo());
		}
		default:
			break;
		}
		return false;
	}

	
};

//衝突判定クラス（球）
class ModelCollisionSphere : public ModelCollision
{
private:

	//衝突判定情報
	std::vector<DirectX::BoundingSphere*> m_srcSpheres;

	//衝突判定情報（ワールド空間）
	std::vector<DirectX::BoundingSphere> m_dstSpheres;

public:

	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="pModel">コリジョン生成の元になる3Dモデルデータへのポインタ</param>
	ModelCollisionSphere(DirectX::Model* pModel);

	/// <summary>
	/// 衝突判定情報の更新
	/// </summary>
	/// <param name="position">モデルの現在のワールド座標</param>
	/// <param name="rotate">モデルの現在の回転姿勢</param>
	/// <param name="scale">モデルの現在のスケール値</param>
	void UpdateBoundingInfo(DirectX::SimpleMath::Vector3 position, DirectX::SimpleMath::Quaternion rotate, DirectX::SimpleMath::Vector3 scale) override;

	/// <summary>
	/// 境界情報
	/// </summary>
	/// <param name="world">モデルに適用されるワールド変換行列</param>
	void UpdateBoundingInfo(const DirectX::SimpleMath::Matrix& world);


	/// <summary>
	/// 衝突判定を行う関数
	/// </summary>
	/// <param name="collision">判定対象の相手コリジョンへのポインタ</param>
	/// <returns>衝突していればtrue、していなければfalse</returns>
	bool Intersects(ModelCollision* collision) override;

	/// <summary>
	/// 内包判定を行う関数
	/// </summary>
	/// <param name="collision">判定対象の相手コリジョンへのポインタ</param>
	/// <returns>完全に内包していればtrue、していなければfalse</returns>
	bool Contains(ModelCollision* collision) override;

	/// <summary>
	/// 衝突判定情報取得関数
	/// </summary>
	/// <returns>ワールド空間に変換済みの衝突判定情報のリスト</returns>
	const std::vector<DirectX::BoundingSphere>& GetBoundingInfo() { return m_dstSpheres; }

	/// <summary>
	/// 衝突判定の表示に登録する関数
	/// </summary>
	/// <param name="displayCollision">登録先となるデバッグ表示用クラスのポインタ</param>
	void AddDisplayCollision(DisplayCollision* displayCollision);

};

//衝突判定クラス（AABB）
class ModelCollisionBox : public ModelCollision
{
private:

	//衝突判定情報
	std::vector<DirectX::BoundingBox*> m_srcBoxes;

	//衝突判定情報（ワールド空間）
	std::vector<DirectX::BoundingBox> m_dstBoxes;

public:

	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="pModel">コリジョン生成の元になる3Dモデルデータへのポインタ</param>
	ModelCollisionBox(DirectX::Model* pModel);

	/// <summary>
	/// 衝突判定情報の更新
	/// </summary>
	/// <param name="position">モデルの現在のワールド座標</param>
	/// <param name="rotate">モデルの現在の回転姿勢</param>
	/// <param name="scale">モデルの現在のスケール値</param>
	void UpdateBoundingInfo(DirectX::SimpleMath::Vector3 position, DirectX::SimpleMath::Quaternion rotate, DirectX::SimpleMath::Vector3 scale) override;

	/// <summary>
	/// 境界情報
	/// </summary>
	/// <param name="world">モデルに適用されるワールド変換行列</param>
	void UpdateBoundingInfo(const DirectX::SimpleMath::Matrix& world);


	/// <summary>
	/// 衝突判定を行う関数
	/// </summary>
	/// <param name="collision">判定対象の相手コリジョンへのポインタ</param>
	/// <returns>衝突していればtrue、していなければfalse</returns>
	bool Intersects(ModelCollision* collision) override;

	/// <summary>
	/// 内包判定を行う関数
	/// </summary>
	/// <param name="collision">判定対象の相手コリジョンへのポインタ</param>
	/// <returns>完全に内包していればtrue、していなければfalse</returns>
	bool Contains(ModelCollision* collision) override;

	/// <summary>
	/// 衝突判定情報取得関数
	/// </summary>
	/// <returns>ワールド空間に変換済みの衝突判定情報のリスト</returns>
	const std::vector<DirectX::BoundingBox>& GetBoundingInfo() { return m_dstBoxes; }

	/// <summary>
	/// 衝突判定の表示に登録する関数
	/// </summary>
	/// <param name="displayCollision">登録先となるデバッグ表示用クラスのポインタ</param>
	void AddDisplayCollision(DisplayCollision* displayCollision);

};

//衝突判定クラス（OBB）
class ModelCollisionOrientedBox : public ModelCollision
{
private:

	//衝突判定情報
	std::vector<DirectX::BoundingBox*> m_srcBoxes;

	//衝突判定情報（ワールド空間）
	std::vector<DirectX::BoundingOrientedBox> m_dstOrientedBoxes;

	//OBBの　ローカル中心座標
	DirectX::SimpleMath::Vector3 m_center;

	//OBBのローカルサイズ
	DirectX::SimpleMath::Vector3 m_extents;

public:

	/// <summary>
	/// デフォルトコンストラクタ
	/// </summary>
	ModelCollisionOrientedBox();

	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="pModel">コリジョン生成の元になる3Dモデルデータへのポインタ</param>
	ModelCollisionOrientedBox(DirectX::Model* pModel);

	/// <summary>
	/// 衝突判定情報の更新
	/// </summary>
	/// <param name="position">モデルのワールド座標</param>
	/// <param name="rotate">モデルの回転</param>
	/// <param name="scale">モデルのスケール</param>
	void UpdateBoundingInfo(DirectX::SimpleMath::Vector3 position, DirectX::SimpleMath::Quaternion rotate, DirectX::SimpleMath::Vector3 scale) override;

	/// <summary>
	/// 境界情報
	/// </summary>
	/// <param name="world">モデルに適用されるワールド変換行列</param>
	void UpdateBoundingInfo(const DirectX::SimpleMath::Matrix& world);

	/// <summary>
	/// 衝突判定を行う関数
	/// </summary>
	/// <param name="collision">判定対象の相手コリジョンへのポインタ</param>
	/// <returns>衝突していればtrue、していなければfalse</returns>
	bool Intersects(ModelCollision* collision) override;

	/// <summary>
	/// 内包判定を行う関数
	/// </summary>
	/// <param name="collision">判定対象の相手コリジョンへのポインタ</param>
	/// <returns>完全に内包していればtrue、していなければfalse</returns>
	bool Contains(ModelCollision* collision) override;

	/// <summary>
	/// 衝突判定情報取得関数
	/// </summary>
	/// <returns>ワールド空間に変換済みの衝突判定情報のリスト</returns>
	const std::vector<DirectX::BoundingOrientedBox>& GetBoundingInfo() { return m_dstOrientedBoxes; }

	/// <summary>
	/// 衝突判定の表示に登録する関数
	/// </summary>
	/// <param name="displayCollision">登録先となるデバッグ表示用クラスのポインタ</param>
	void AddDisplayCollision(DisplayCollision* displayCollision);

	/// <summary>
	/// 中心座標を直接設定
	/// </summary>
	/// <param name="center">設定する中心座標</param>
	void SetCenter(const DirectX::SimpleMath::Vector3& center);
	
	/// <summary>
	/// サイズを直接設定
	/// </summary>
	/// <param name="extents">設定するサイズ</param>
	void SetExtents(const DirectX::SimpleMath::Vector3& extents);

	/// <summary>
	/// 現在の中心座標を取得
	/// </summary>
	/// <returns>中心座標</returns>
	DirectX::SimpleMath::Vector3 GetCenter() const;
	
	/// <summary>
	/// 現在のサイズを取得
	/// </summary>
	/// <returns>サイズ</returns>
	DirectX::SimpleMath::Vector3 GetExtents() const;

};

//衝突判定生成クラス
class ModelCollisionFactory
{
public:

	/// <summary>
	/// 指定したタイプとモデルデータに基づいて、適切な衝突判定オブジェクトを生成
	/// </summary>
	/// <param name="type">生成するコリジョンの形状タイプ</param>
	/// <param name="pModel">衝突判定の元となるモデルデータへのポインタ</param>
	/// <returns>生成された衝突判定オブジェクトのユニークポインタ</returns>
	static std::unique_ptr<ModelCollision> CreateCollision(ModelCollision::CollisionType type, DirectX::Model* pModel)
	{
		std::unique_ptr<ModelCollision> collision;

		switch (type)
		{
		case ModelCollision::CollisionType::Sphere:
			collision = std::make_unique<ModelCollisionSphere>(pModel);
			break;
		case ModelCollision::CollisionType::AABB:
			collision = std::make_unique<ModelCollisionBox>(pModel);
			break;
		case ModelCollision::CollisionType::OBB:
			collision = std::make_unique<ModelCollisionOrientedBox>(pModel);
			break;
		default:
			break;
		}

		return collision;
	}
};

