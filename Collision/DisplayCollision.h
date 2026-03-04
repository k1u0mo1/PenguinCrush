
//DisplayCollision.h
//コリジョン表示

#pragma once


#include <vector>
#include "SimpleMath.h"
#include "CommonStates.h"
#include "Effects.h"
#include "GeometricPrimitive.h"
#include "DirectXHelpers.h"
#include "PrimitiveBatch.h"
#include "VertexTypes.h"

/// <summary>
/// デバッグ用のコリジョンを画面に描画するクラス
/// </summary>
class DisplayCollision
{
private:

	//表示可能のコリジョンの最大　デフォルト
	static const uint32_t DISPLAY_COLLISION_MAX = 100;

	//表示可能のコリジョンの最大
	uint32_t m_collisionMax;

	//モデルの表示
	bool m_modelActive;

	// ラインの表示
	bool m_lineActive;

	// 球の情報
	struct Sphere
	{
		//中心
		DirectX::SimpleMath::Vector3 center;	
		//半径
		float radius;							
		//色（ライン用）
		DirectX::SimpleMath::Color lineColor;	

		constexpr Sphere(
			const DirectX::SimpleMath::Vector3& center,
			float radius,
			DirectX::SimpleMath::Color lineColor) noexcept
			: center(center), radius(radius), lineColor(lineColor) {}
		
	};

	// ボックスの情報
	struct Box
	{
		//中心
		DirectX::SimpleMath::Vector3 center;
		//各面の中心からの距離.
		DirectX::SimpleMath::Vector3 extents;
		//回転
		DirectX::SimpleMath::Quaternion rotate;	
		//色（ライン用）
		DirectX::SimpleMath::Color lineColor;	

		constexpr Box(
			const DirectX::SimpleMath::Vector3& center,
			const DirectX::SimpleMath::Vector3& extents,
			const DirectX::SimpleMath::Quaternion& rotate,
			DirectX::SimpleMath::Color lineColor) noexcept
			: center(center), extents(extents), rotate(rotate), lineColor(lineColor) {}
	};
	 
	//球のコリジョン情報
	std::vector<Sphere> m_spheres;

	//ボックスのコリジョン情報
	std::vector<Box> m_boxes;

	//球のモデル
	std::unique_ptr<DirectX::GeometricPrimitive> m_modelSphere;

	//ボックスのモデル
	std::unique_ptr<DirectX::GeometricPrimitive> m_modelBox;

	//エフェクト（モデル用）
	std::unique_ptr<DirectX::NormalMapEffect> m_modelEffect;

	//入力レイアウト（モデル用）
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_modelInputLayout;

	//インスタンス用頂点バッファ
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_instancedVB;

	//プリミティブバッチ（ライン用）
	std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>> m_primitiveBatch;

	//エフェクト（ライン用）
	std::unique_ptr<DirectX::BasicEffect> m_lineEffect;

	//入力レイアウト（ライン用）
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_lineInputLayout;

public:

	//表示切り替え用の関数
	void SetVisible(bool visible) { m_isVisible = visible; }

private:

	//表示フラグ
	bool m_isVisible = true;

private:

	//コリジョンモデルの描画関数
	void DrawCollisionModel(
		ID3D11DeviceContext* context,
		DirectX::CommonStates* states,
		const DirectX::SimpleMath::Matrix& view,
		const DirectX::SimpleMath::Matrix& proj,
		DirectX::FXMVECTOR color
	);

	//コリジョンラインの描画関数
	void DrawCollisionLine(
		ID3D11DeviceContext* context,
		DirectX::CommonStates* states,
		const DirectX::SimpleMath::Matrix& view,
		const DirectX::SimpleMath::Matrix& proj,
		DirectX::FXMVECTOR color
	);

public:

	//コンストラクタ
	DisplayCollision(
		ID3D11Device* device,
		ID3D11DeviceContext* context,
		bool modelActive = true,
		bool lineActive = true,
		uint32_t collisionMax = DISPLAY_COLLISION_MAX
	);
	
	//登録されたコリジョンの描画関数
	void DrawCollision(
		ID3D11DeviceContext* context,
		DirectX::CommonStates* states,
		const DirectX::SimpleMath::Matrix& view,
		const DirectX::SimpleMath::Matrix& proj,
		DirectX::FXMVECTOR baseColor = DirectX::Colors::White,
		DirectX::FXMVECTOR lineColor = DirectX::XMVECTORF32{ 0.0f, 0.0f, 0.0f, 0.0f },
		float alpha = 0.5f
	);

	//球のコリジョンを登録する関数
	void AddBoundingSphere(
		DirectX::BoundingSphere shpere,
		DirectX::FXMVECTOR lineColor = DirectX::XMVECTORF32{ 0.0f, 0.0f, 0.0f, 0.0f })
	{
		DirectX::XMFLOAT3 center = shpere.Center;
		m_spheres.push_back(Sphere(center, shpere.Radius, lineColor));
	}


	//ボックスのコリジョンを登録する関数
	void AddBoundingBox(
		DirectX::BoundingBox box,
		DirectX::FXMVECTOR lineColor = DirectX::XMVECTORF32{ 0.0f, 0.0f, 0.0f, 0.0f })
	{
		m_boxes.push_back(Box(box.Center, box.Extents, DirectX::SimpleMath::Quaternion(), lineColor));
	}

	//回転したボックスのコリジョンを登録する関数
	void AddBoundingOrientedBox(
		DirectX::BoundingOrientedBox box,
		DirectX::FXMVECTOR lineColor = DirectX::XMVECTORF32{ 0.0f, 0.0f, 0.0f, 0.0f })
	{
		m_boxes.push_back(Box(box.Center, box.Extents, DirectX::SimpleMath::Quaternion(box.Orientation), lineColor));
	}

	//コリジョンモデルの表示（ON/OFF）
	void SetModelActive(bool active) { m_modelActive = active; }

	//コリジョンラインの表示（ON/OFF）
	void SetLineActive(bool active) { m_lineActive = active; }


};

