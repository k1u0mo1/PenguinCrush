
//Particle.h
//シェーダ用のパーティクルクラス

#pragma once

#include "Common/StepTimer.h"
#include <Common/DeviceResources.h>
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


	struct ConstBuffer
	{
		DirectX::SimpleMath::Matrix		matWorld;
		DirectX::SimpleMath::Matrix		matView;
		DirectX::SimpleMath::Matrix		matProj;
		DirectX::SimpleMath::Vector4	Diffuse;
		DirectX::SimpleMath::Vector4	time;
	};

	

private:

	//	変数
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
	//	関数
	static const std::vector<D3D11_INPUT_ELEMENT_DESC> INPUT_LAYOUT;

	//コンストラクタ
	Particle();
	//デストラクタ
	~Particle();

	//ロードテクスチャ
	void LoadTexture(const wchar_t* path);

	void Create(DX::DeviceResources* pDR);

	//更新
	void Update(float elapsedTime);

	////水しぶきを発生させる関数
	//void SpawnSplash(const DirectX::SimpleMath::Vector3& position, int count = 20);

	//タイプの切り替え
	void Spawn(Type type, const DirectX::SimpleMath::Vector3& position, int count = 20,float size=1.0f);

	//描画
	void Render(DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix proj);

	
	
private:

	void CreateShader();
};

