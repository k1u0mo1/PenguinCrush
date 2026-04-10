
//Particle.h
//シェーダ用のパーティクルクラス

#pragma once

#include "Game/Common/StepTimer.h"
#include "Game/Common/DeviceResources.h"
#include <SimpleMath.h>
#include <Effects.h>
#include <PrimitiveBatch.h>
#include <VertexTypes.h>
#include <WICTextureLoader.h>
#include <CommonStates.h>
#include <vector>

#include <algorithm>

class Particle
{
public:

	//エフェクトの情報　切り替え用
	enum class Type
	{
		//落ちたときの水しぶき
		Splash,
		//攻撃が当たった時の爆発
		Explosion,
		//ダッシュ中の後ろから線
		Dash
	};

	//パーティクル1粒の情報
	struct ParticleInfo
	{
		//位置
		DirectX::SimpleMath::Vector3 Position;
		//速度
		DirectX::SimpleMath::Vector3 Velocity;
		//色
		DirectX::SimpleMath::Vector4 Color;

		//経過時間
		float Age;
		//寿命
		float Lifetime;
		//サイズ
		float Size;

	};

	/// <summary>
	/// シェーダに渡す定数バッファ用構造体
	/// </summary>
	struct ConstBuffer
	{
		//ワールド変換行 
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

	Microsoft::WRL::ComPtr<ID3D11Buffer>	m_CBuffer;

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

	//パーティクルのリスト 要らないかも
	std::vector<ParticleInfo> m_particles;
	
	//パーティクル関係
	//水しぶき用
	std::vector<ParticleInfo> m_splashList;
	//攻撃が当たったあとの爆発
	std::vector<ParticleInfo> m_explosionList;
	//ダッシュ中の線
	std::vector<ParticleInfo> m_dashList;

public:
	//	頂点シェーダへ渡す頂点データの入力レイアウト定義
	static const std::vector<D3D11_INPUT_ELEMENT_DESC> INPUT_LAYOUT;

	/// <summary>
	/// コンストラクタ
	/// </summary>
	Particle();
	
	/// <summary>
	/// デストラクタ
	/// </summary>
	~Particle();

	/// <summary>
	/// 演出に使用するテクスチャを読み込む
	/// </summary>
	/// <param name="path">テクスチャファイルのパス</param>
	void LoadTexture(const wchar_t* path);

	/// <summary>
	/// デバイスリソースの初期化
	/// </summary>
	/// <param name="pDR">DeviceResourcesへのポインタ</param>
	void Initialize(DX::DeviceResources* pDR);

	/// <summary>
	/// パーティクルの移動・寿命計算などの更新処理
	/// </summary>
	/// <param name="elapsedTime">ステップタイマー</param>
	void Update(float elapsedTime);

	/// <summary>
	/// 指定した座標にパーティクルを発生
	/// </summary>
	/// <param name="type">発生させるエフェクトの種類</param>
	/// <param name="position">発生させる中心座標</param>
	/// <param name="count">エフェクトの数</param>
	/// <param name="size">エフェクトのサイズ</param>
	void Spawn(Type type, const DirectX::SimpleMath::Vector3& position, int count = 20,float size=1.0f);

	/// <summary>
	/// 3D空間へのパーティクル描画処理
	/// </summary>
	/// <param name="view">ビュー行列</param>
	/// <param name="proj">射影行列</param>
	void Render(DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix proj);
	
	/// <summary>
	/// パーティクル描画用のシェーダとインプットレイアウトを作成
	/// </summary>
	void CreateShader();
};

