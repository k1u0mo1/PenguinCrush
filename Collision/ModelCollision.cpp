
//Collision.cpp
//モデル衝突判定

#include "pch.h"
#include "ModelCollision.h"

using namespace DirectX;

bool ModelCollision::Intersects(const ModelCollision& other) const
{
	return m_boundingOrientedBox.Intersects(other.m_boundingOrientedBox);
}

DirectX::SimpleMath::Vector3 ModelCollision::GetAABBMin() const
{
	return DirectX::SimpleMath::Vector3();
}

DirectX::SimpleMath::Vector3 ModelCollision::GetAABBMax() const
{
	return DirectX::SimpleMath::Vector3();
}

//ModelCollision::ModelCollision(const DirectX::SimpleMath::Vector3& size)
//{
//}




const DirectX::SimpleMath::Vector3& ModelCollision::GetCenter() const
{
	return m_boundingOrientedBox.Center;
}

const DirectX::SimpleMath::Vector3& ModelCollision::GetExtents() const
{
	return m_boundingOrientedBox.Extents;
}

// コンストラクタ
ModelCollision::ModelCollision(CollisionType type)
	: m_type(type)
{
}

// ------------------------------------------------------------------------------- //

// コンストラクタ
ModelCollisionSphere::ModelCollisionSphere(DirectX::Model* pModel)
	: ModelCollision(ModelCollision::CollisionType::Sphere)
{
	// コリジョン情報をモデルデータから取得
	for (auto& mesh : pModel->meshes) {
		m_srcSpheres.push_back(&mesh->boundingSphere);
	}
	// ワールド空間内のコリジョン情報を保存するワークを確保する
	m_dstSpheres.resize(pModel->meshes.size());
}

// 衝突判定情報の更新
void ModelCollisionSphere::UpdateBoundingInfo(
	DirectX::SimpleMath::Vector3 position,
	DirectX::SimpleMath::Quaternion rotate,
	DirectX::SimpleMath::Vector3 /*scale*/
)
{
	UNREFERENCED_PARAMETER(rotate);

	for (int i = 0; i < m_srcSpheres.size(); i++)
	{
		DirectX::BoundingSphere dst(*m_srcSpheres[i]);
		dst.Center = DirectX::SimpleMath::Vector3::Transform(m_srcSpheres[i]->Center, rotate) + position;
		m_dstSpheres[i] = dst;
	}
}

void ModelCollisionSphere::UpdateBoundingInfo(const DirectX::SimpleMath::Matrix& world)
{
	for (int i = 0; i < m_srcSpheres.size(); i++)
	{
		DirectX::BoundingSphere dst(*m_srcSpheres[i]);
		dst.Transform(dst, world);
		m_dstSpheres[i] = dst;
	}
}

// 衝突判定を行う関数
bool ModelCollisionSphere::Intersects(ModelCollision* collision)
{
	return ModelCollision::Intersects<ModelCollisionSphere, DirectX::BoundingSphere>(this, collision);
}

// 内包判定を行う関数
bool ModelCollisionSphere::Contains(ModelCollision* collision)
{
	return ModelCollision::Contains<ModelCollisionSphere, DirectX::BoundingSphere>(this, collision);
}

// 衝突判定の表示に登録する関数
void ModelCollisionSphere::AddDisplayCollision(DisplayCollision* displayCollision)
{
	for (auto& data : GetBoundingInfo())
	{
		displayCollision->AddBoundingSphere(data);
	}
}

// ------------------------------------------------------------------------------- //

// コンストラクタ
ModelCollisionBox::ModelCollisionBox(DirectX::Model* pModel)
	: ModelCollision(ModelCollision::CollisionType::AABB)
{
	// コリジョン情報をモデルデータから取得
	for (auto& mesh : pModel->meshes) {
		m_srcBoxes.push_back(&mesh->boundingBox);
	}
	// ワールド空間内のコリジョン情報を保存するワークを確保する
	m_dstBoxes.resize(pModel->meshes.size());
}

// 衝突判定情報の更新
void ModelCollisionBox::UpdateBoundingInfo(
	DirectX::SimpleMath::Vector3 position,
	DirectX::SimpleMath::Quaternion rotate,
	DirectX::SimpleMath::Vector3 /*scale*/
)
{
	UNREFERENCED_PARAMETER(rotate);

	for (int i = 0; i < m_srcBoxes.size(); i++)
	{
		DirectX::BoundingBox dst(*m_srcBoxes[i]);
		dst.Center = DirectX::SimpleMath::Vector3::Transform(m_srcBoxes[i]->Center, rotate) + position;
		m_dstBoxes[i] = dst;
	}
}

void ModelCollisionBox::UpdateBoundingInfo(const DirectX::SimpleMath::Matrix& world)
{
	for (int i = 0; i < m_srcBoxes.size(); i++)
	{
		DirectX::BoundingBox dst(*m_srcBoxes[i]);
		dst.Transform(dst, world);
		m_dstBoxes[i] = dst;
	}
}

// 衝突判定を行う関数
bool ModelCollisionBox::Intersects(ModelCollision* collision)
{
	return ModelCollision::Intersects<ModelCollisionBox, DirectX::BoundingBox>(this, collision);
}

// 内包判定を行う関数
bool ModelCollisionBox::Contains(ModelCollision* collision)
{
	return ModelCollision::Contains<ModelCollisionBox, DirectX::BoundingBox>(this, collision);
}

// 衝突判定の表示に登録する関数
void ModelCollisionBox::AddDisplayCollision(DisplayCollision* displayCollision)
{
	for (auto& data : GetBoundingInfo())
	{
		displayCollision->AddBoundingBox(data);
	}
}

// ------------------------------------------------------------------------------- //

DirectX::SimpleMath::Vector3 ModelCollisionOrientedBox::GetCenter() const
{
	if (!m_dstOrientedBoxes.empty())
		return m_dstOrientedBoxes[0].Center;
	return DirectX::SimpleMath::Vector3::Zero;
}

DirectX::SimpleMath::Vector3 ModelCollisionOrientedBox::GetExtents() const
{
	if (!m_dstOrientedBoxes.empty())
		return m_dstOrientedBoxes[0].Extents;
	return DirectX::SimpleMath::Vector3::Zero;
}

ModelCollisionOrientedBox::ModelCollisionOrientedBox()
	: ModelCollision(ModelCollision::CollisionType::OBB)
{
	DirectX::BoundingOrientedBox box;
	box.Center = DirectX::SimpleMath::Vector3::Zero;
	box.Extents = DirectX::SimpleMath::Vector3(0.5f, 0.5f, 0.5f); // デフォルトサイズ
	box.Orientation = DirectX::SimpleMath::Quaternion::Identity;

	m_dstOrientedBoxes.push_back(box);
}

// コンストラクタ
ModelCollisionOrientedBox::ModelCollisionOrientedBox(DirectX::Model* pModel)
	: ModelCollision(ModelCollision::CollisionType::OBB)
{
	// コリジョン情報をモデルデータから取得
	for (auto& mesh : pModel->meshes) {
		m_srcBoxes.push_back(&mesh->boundingBox);
	}
	// ワールド空間内のコリジョン情報を保存するワークを確保する
	m_dstOrientedBoxes.resize(pModel->meshes.size());
}

// 衝突判定情報の更新
void ModelCollisionOrientedBox::UpdateBoundingInfo(
	DirectX::SimpleMath::Vector3 position,
	DirectX::SimpleMath::Quaternion rotate,
	DirectX::SimpleMath::Vector3 /*scale*/
)
{
	for (int i = 0; i < m_srcBoxes.size(); i++)
	{
		DirectX::BoundingOrientedBox dst;
		DirectX::BoundingOrientedBox::CreateFromBoundingBox(dst, *m_srcBoxes[i]);

		dst.Center = DirectX::SimpleMath::Vector3::Transform(m_srcBoxes[i]->Center, rotate) + position;

		dst.Orientation = rotate;
		m_dstOrientedBoxes[i] = dst;

	}
}

//ステージの大きさを揃える用
void ModelCollisionOrientedBox::UpdateBoundingInfo(
	const DirectX::SimpleMath::Matrix& world)
{
	if (m_srcBoxes.empty() || m_dstOrientedBoxes.size() < m_srcBoxes.size())
		return; // 安全にスキップ

	for (int i = 0; i < m_srcBoxes.size(); i++)
	{
		DirectX::BoundingOrientedBox dst;
		DirectX::BoundingOrientedBox::CreateFromBoundingBox(dst, *m_srcBoxes[i]);

		// 行列で OBB を変換
		dst.Transform(dst, world);

		m_dstOrientedBoxes[i] = dst;
	}
}

// 衝突判定を行う関数
bool ModelCollisionOrientedBox::Intersects(ModelCollision* collision)
{
	return ModelCollision::Intersects<ModelCollisionOrientedBox, DirectX::BoundingOrientedBox>(this, collision);
}

// 内包判定を行う関数
bool ModelCollisionOrientedBox::Contains(ModelCollision* collision)
{
	return ModelCollision::Contains<ModelCollisionOrientedBox, DirectX::BoundingOrientedBox>(this, collision);
}

// 衝突判定の表示に登録する関数
void ModelCollisionOrientedBox::AddDisplayCollision(DisplayCollision* displayCollision)
{
	for (auto& data : GetBoundingInfo())
	{
		displayCollision->AddBoundingOrientedBox(data);
	}
}

void ModelCollisionOrientedBox::SetCenter(const DirectX::SimpleMath::Vector3& center)
{
	for (auto& box : m_dstOrientedBoxes)
	{
		box.Center = center;
	}
}

void ModelCollisionOrientedBox::SetExtents(const DirectX::SimpleMath::Vector3& extents)
{
	for (auto& box : m_dstOrientedBoxes)
	{
		box.Extents = extents;
	}
}
