
//ModelCollision.h
//モデルの衝突判定

#pragma once
#include "Collision/DisplayCollision.h"
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

	bool Intersects(const ModelCollision& other) const;

	DirectX::SimpleMath::Vector3 GetAABBMin() const;
	DirectX::SimpleMath::Vector3 GetAABBMax() const;

	//ModelCollision(const DirectX::SimpleMath::Vector3& size); // size = extents * 2

	const DirectX::SimpleMath::Vector3& GetCenter() const;
	const DirectX::SimpleMath::Vector3& GetExtents() const;

private:

	//コリジョンタイプ
	CollisionType m_type;

	DirectX::BoundingOrientedBox m_boundingOrientedBox;

public:

	//コンストラクタ
	ModelCollision(CollisionType type);

	//デストラクタ
	virtual ~ModelCollision() {}

	//衝突判定を行う関数
	virtual bool Intersects(ModelCollision* collision) = 0;

	//内包判定を行う関数
	virtual bool Contains(ModelCollision* collision) = 0;

	//コリジョンタイプの取得関数
	CollisionType GetType() const { return m_type; }

	//コリジョン情報の更新
	virtual void UpdateBoundingInfo(
		DirectX::SimpleMath::Vector3 position,
		DirectX::SimpleMath::Quaternion rotate,
		DirectX::SimpleMath::Vector3 scale =
		DirectX::SimpleMath::Vector3(1.0f, 1.0f, 1.0f)) = 0;

	//衝突判定の表示に登録する関数
	virtual void AddDisplayCollision(DisplayCollision* displayCollision) = 0;

	virtual void UpdateBoundingInfo(const DirectX::SimpleMath::Matrix& world) = 0;

protected:

	//衝突判定（複数メッシュに対応）
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

	//衝突判定
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

	//内包判定
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

	// 内包判定
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

	//コンストラクタ
	ModelCollisionSphere(DirectX::Model* pModel);

	//衝突判定情報の更新
	void UpdateBoundingInfo(DirectX::SimpleMath::Vector3 position, DirectX::SimpleMath::Quaternion rotate, DirectX::SimpleMath::Vector3 scale) override;

	void UpdateBoundingInfo(const DirectX::SimpleMath::Matrix& world);


	//衝突判定を行う関数
	bool Intersects(ModelCollision* collision) override;

	//内包判定を行う関数
	bool Contains(ModelCollision* collision) override;

	//衝突判定情報取得関数
	const std::vector<DirectX::BoundingSphere>& GetBoundingInfo() { return m_dstSpheres; }

	//衝突判定の表示に登録する関数
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

	//コンストラクタ
	ModelCollisionBox(DirectX::Model* pModel);

	//衝突判定情報の更新
	void UpdateBoundingInfo(DirectX::SimpleMath::Vector3 position, DirectX::SimpleMath::Quaternion rotate, DirectX::SimpleMath::Vector3 scale) override;

	void UpdateBoundingInfo(const DirectX::SimpleMath::Matrix& world);


	//衝突判定を行う関数
	bool Intersects(ModelCollision* collision) override;

	//内包判定を行う関数
	bool Contains(ModelCollision* collision) override;

	//衝突判定情報取得関数
	const std::vector<DirectX::BoundingBox>& GetBoundingInfo() { return m_dstBoxes; }

	//衝突判定の表示に登録する関数
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


	DirectX::SimpleMath::Vector3 m_center;
	DirectX::SimpleMath::Vector3 m_extents;

public:

	// デフォルトコンストラクタ
	ModelCollisionOrientedBox();

	// コンストラクタ
	ModelCollisionOrientedBox(DirectX::Model* pModel);

	// 衝突判定情報の更新
	void UpdateBoundingInfo(DirectX::SimpleMath::Vector3 position, DirectX::SimpleMath::Quaternion rotate, DirectX::SimpleMath::Vector3 scale) override;

	void UpdateBoundingInfo(const DirectX::SimpleMath::Matrix& world);

	// 衝突判定を行う関数
	bool Intersects(ModelCollision* collision) override;

	// 内包判定を行う関数
	bool Contains(ModelCollision* collision) override;

	// 衝突判定情報取得関数
	const std::vector<DirectX::BoundingOrientedBox>& GetBoundingInfo() { return m_dstOrientedBoxes; }

	// 衝突判定の表示に登録する関数
	void AddDisplayCollision(DisplayCollision* displayCollision);

	// 中心とサイズを直接設定
	void SetCenter(const DirectX::SimpleMath::Vector3& center);
	void SetExtents(const DirectX::SimpleMath::Vector3& extents);

	// 現在の中心・サイズを取得
	DirectX::SimpleMath::Vector3 GetCenter() const;
	DirectX::SimpleMath::Vector3 GetExtents() const;

};

//衝突判定生成クラス
class ModelCollisionFactory
{
public:

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

