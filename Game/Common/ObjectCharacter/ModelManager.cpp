
/**
 * @file   ModelManager.h
 * @brief  キャラクターのモデルを一括で管理管理を行うクラス
 * @author 國田知睦
 * @date   2026/06/08
 */

#include "pch.h"
#include "ModelManager.h"

#include <Effects.h>

void ModelManager::Initialize(ID3D11Device* device)
{

	DirectX::EffectFactory fx(device);
	fx.SetDirectory(L"Resources\\Models");

	//各状態のモデルをロード
	m_modelIdle     = DirectX::Model::CreateFromSDKMESH(device, L"Resources\\Models\\Pen_Stand.sdkmesh", fx);
	m_modelAttack   = DirectX::Model::CreateFromSDKMESH(device, L"Resources\\Models\\PenAttack.sdkmesh", fx);
	m_modelRush     = DirectX::Model::CreateFromSDKMESH(device, L"Resources\\Models\\Rush.sdkmesh",      fx);
	m_materialDizzy = DirectX::Model::CreateFromSDKMESH(device, L"Resources\\Models\\Fainting.sdkmesh",  fx);
}