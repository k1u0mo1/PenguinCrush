/**
 * @file   SkyDome.cpp
 * @brief  スカイドームの描画・管理を行うクラス
 * @author 國田知睦
 * @date   2026/07/01
 */

#include "pch.h"
#include "Game/ObjectList/SkyDome.h"
#include <Effects.h>

//-----------------------------------------------------------------
// コンストラクタ
//-----------------------------------------------------------------

SkyDome::SkyDome(DX::DeviceResources* deviceResources)
	: 
	m_deviceResources(deviceResources)
{
}

//-----------------------------------------------------------------
// モデルの初期化
//-----------------------------------------------------------------

void SkyDome::Initialize()
{
	auto device = m_deviceResources->GetD3DDevice();

	//CommonStatesを初期化
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
	
	//明るさ設定
	m_model->UpdateEffects([&](DirectX::IEffect* effect)
		{
			auto lights = dynamic_cast<DirectX::IEffectLights*>(effect);
			if (lights)
			{
				lights->SetLightingEnabled(false);
			}
		}
	);

	m_position= { 0.0f, HEIGHT_OFFSET_Y, 0.0f };

}

//-----------------------------------------------------------------
// モデルの描画
//-----------------------------------------------------------------

void SkyDome::Render(
	ID3D11DeviceContext* context,
	const DirectX::SimpleMath::Matrix& view,
	const DirectX::SimpleMath::Matrix& proj)
{
	if (!m_model || !m_states) return;

	//固定する
	DirectX::SimpleMath::Matrix world
		= DirectX::SimpleMath::Matrix::CreateScale(DEFAULT_SCALE)
		* DirectX::SimpleMath::Matrix::CreateTranslation(m_position);
	
	//ステートの設定
	context->OMSetDepthStencilState(m_states->DepthNone(), 0);
	//裏側カリングを無効
	context->RSSetState(m_states->CullNone());

	//描画する直前にライトをもう一度OFFにする
	m_model->UpdateEffects([&](DirectX::IEffect* effect)
		{
			auto lights = dynamic_cast<DirectX::IEffectLights*>(effect);
			if (lights)
			{
				lights->SetLightingEnabled(false);
			}
		});

	context->VSSetShader(nullptr, nullptr, 0);
	context->PSSetShader(nullptr, nullptr, 0);

	for (const auto& mesh : m_model->meshes)
	{
		// モデルの描画（カスタムステートでサンプラーをリニアに強制する）
		mesh->Draw(context, world, view, proj, true, [this, context]()
			{
				// サンプラーステートを滑らかな補間（LinearWrap）に設定
				ID3D11SamplerState* sampler = m_states->LinearWrap();
				context->PSSetSamplers(0, 1, &sampler);
			});
	}

	//ステートに戻す
	context->OMSetDepthStencilState(m_states->DepthDefault(), 0);
	context->RSSetState(m_states->CullCounterClockwise());
}
