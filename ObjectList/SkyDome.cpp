//
// スカイドーム（背景）
//

#include "pch.h"
#include "ObjectList/SkyDome.h"
#include <Effects.h>

SkyDome::SkyDome(DX::DeviceResources* deviceResources)
	: m_deviceResources(deviceResources)
{
}

void SkyDome::Initialize()
{
	auto device = m_deviceResources->GetD3DDevice();

	//Commonstatesを初期化
	m_states = std::make_unique<DirectX::CommonStates>(device);

	//エフェクトファクトリを設定
	m_effectFactory = std::make_unique<DirectX::EffectFactory>(device);
	m_effectFactory->SetDirectory(L"Resources/Models");

	//モデルをロード
	m_model = DirectX::Model::CreateFromSDKMESH(
		device,
		L"Resources\\Models\\SkyDome.sdkmesh",
		*m_effectFactory
	);

	////明るさ設定
	//m_model->UpdateEffects([&](DirectX::IEffect* effect)
	//	{
	//		auto lights = dynamic_cast<DirectX::IEffectLights*>(effect);
	//		if (lights)
	//		{
	//			lights->SetLightingEnabled(false);
	//		}
	//	}
	//);

	m_position= { 0.0f, -20.0f, 0.0f };

}

void SkyDome::Render(
	ID3D11DeviceContext* context,
	const DirectX::SimpleMath::Matrix& view,
	const DirectX::SimpleMath::Matrix& proj)
{
	if (!m_model) return;

	//固定する
	DirectX::SimpleMath::Matrix world
		= DirectX::SimpleMath::Matrix::CreateScale(m_scale)
		* DirectX::SimpleMath::Matrix::CreateTranslation(m_position);
	
	//ステートの設定
	context->OMSetDepthStencilState(m_states->DepthNone(), 0);
	//裏側カリングを無効
	context->RSSetState(m_states->CullNone());

	//モデルの描画
	m_model->Draw(context, *m_states, world, view, proj);

	//ステートに戻す
	context->OMSetDepthStencilState(m_states->DepthDefault(), 0);
	context->RSSetState(m_states->CullCounterClockwise());

}
