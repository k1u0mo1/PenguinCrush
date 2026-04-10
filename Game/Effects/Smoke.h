
//Smoke.h
//シェーダ用のパーティクルクラス

#pragma once

#include "Game/Common/StepTimer.h"
#include <Game/Common/DeviceResources.h>
#include <SimpleMath.h>
#include <Effects.h>
#include <PrimitiveBatch.h>
#include <VertexTypes.h>
#include <WICTextureLoader.h>
#include <CommonStates.h>
#include <vector>


class Smoke
{
public:

	/// <summary>
	/// シェーダに渡す定数バッファ用構造体
	/// </summary>
	struct ConstBuffer
	{
		//ワールド変換行列
		DirectX::SimpleMath::Matrix		matWorld;
		//ビュー変換行列
		DirectX::SimpleMath::Matrix		matView;
		//射影変換行列
		DirectX::SimpleMath::Matrix		matProj;
		//ディフューズ色
		DirectX::SimpleMath::Vector4	Diffuse;
		//時間パラメータ
		DirectX::SimpleMath::Vector4	time;
	};

private:

	//デバイスリソースへのポインタ
	DX::DeviceResources* m_pDR;

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_CBuffer;

	DX::StepTimer  m_timer;

	//	入力レイアウト
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;

	//	プリミティブバッチ
	std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColorTexture>> m_batch;
	//	コモンステート
	std::unique_ptr<DirectX::CommonStates> m_states;
	//	テクスチャハンドル
	std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> m_texture;
	//	テクスチャハンドル
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture2;
	//	頂点シェーダ
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
	//	ピクセルシェーダ
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
	//	ジオメトリシェーダ
	Microsoft::WRL::ComPtr<ID3D11GeometryShader> m_geometryShader;

	DirectX::SimpleMath::Matrix m_world;
	DirectX::SimpleMath::Matrix m_view;
	DirectX::SimpleMath::Matrix m_proj;

public:
	//頂点シェーダへ渡す頂点データの入力レイアウト定義
	static const std::vector<D3D11_INPUT_ELEMENT_DESC> INPUT_LAYOUT;

	/// <summary>
	/// コンストラクタ
	/// </summary>
	Smoke();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~Smoke();

	/// <summary>
	/// 煙のテクスチャを読み込む
	/// </summary>
	/// <param name="path">テクスチャファイルのパス</param>
	void LoadTexture(const wchar_t* path);

	/// <summary>
	/// 初期化処理
	/// </summary>
	/// <param name="pDR">DeviceResourcesへのポインタ</param>
	void Initialize(DX::DeviceResources* pDR);

	/// <summary>
	/// 煙の描画
	/// </summary>
	/// <param name="view">ビュー行列</param>
	/// <param name="proj">射影行列</param>
	/// <param name="position">描画座標</param>
	/// <param name="scale">エフェクトのサイズ</param>
	/// <param name="alpha">色情報</param>
	void Render(
		DirectX::SimpleMath::Matrix view, 
		DirectX::SimpleMath::Matrix proj,
		const DirectX::SimpleMath::Vector3& position,
		float scale,
		float alpha
	);

	/// <summary>
	/// 煙描画用のシェーダを作成　
	/// </summary>
	void CreateShader();
};

