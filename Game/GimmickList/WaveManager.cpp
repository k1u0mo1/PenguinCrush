

/**
 * @file   WaveManager.cpp
 * @brief  波の生成と描画を管理するマネージャークラス
 * @author 國田知睦
 * @date   2026/07/01
 */

#include "pch.h"
#include "Game/GimmickList/WaveManager.h"

/// 波のモード　trueなら立方体、falseなら通常の波
bool WaveManager::ms_isCubeMode = false;

//----------------------------------------------------------
// コンテキストトを受け取って波のインスタンスを生成
//----------------------------------------------------------

WaveManager::WaveManager(DX::DeviceResources* deviceResources)
{
	m_wave = std::make_unique<Wave>(deviceResources);
}

//----------------------------------------------------------
// 波の初期化
//----------------------------------------------------------

void WaveManager::Initialize(HWND hwnd, int width, int height)
{
	//波の初期化
	m_wave->Initialize(hwnd, width, height);

	//初期化時にモードを反映
	if(ms_isCubeMode)
	{
		m_wave->ToggleDotMode();
	}
}

//----------------------------------------------------------
// 波の更新
//----------------------------------------------------------

void WaveManager::Update(float deltaTime)
{
	//波の更新
	m_wave->Update(deltaTime);	
}

//----------------------------------------------------------
// 波の描画
//----------------------------------------------------------

void WaveManager::Render(
	ID3D11DeviceContext* context,
	const DirectX::SimpleMath::Matrix& view,
	const DirectX::SimpleMath::Matrix& proj)
{
	m_wave->Render(context, view, proj);
}

//----------------------------------------------------------
// 波のモード切り替え
//----------------------------------------------------------

void WaveManager::ToggleMode()
{
	if (m_wave)
	{
		//モードを切り替えるフラグを反転
		ms_isCubeMode = !ms_isCubeMode;
		//モードを切り替える
		m_wave->ToggleDotMode();
	}
}

//----------------------------------------------------------
// 波の角度を取得
//----------------------------------------------------------

DirectX::SimpleMath::Vector2 WaveManager::GetCurrentWaveAngle(float x, float z) const
{
	//波の角度を取得
	return m_wave->GetWaveAngle(x, z);
}

//----------------------------------------------------------
// 波の高さを取得
//----------------------------------------------------------

float WaveManager::GetCurrentHeight(float x, float z) const
{
	//波の高さを取得
	return m_wave->GetHeight(x, z);
}
