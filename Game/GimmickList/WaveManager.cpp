

#include "pch.h"

#include "Game/GimmickList/WaveManager.h"

using namespace DirectX;

/// ”g‚Ìƒ‚[ƒh@true‚È‚ç—§•û‘ÌAfalse‚È‚ç’Êí‚Ì”g
bool WaveManager::ms_isCubeMode = false;

WaveManager::WaveManager(DX::DeviceResources* deviceResources)
{
	m_wave = std::make_unique<Wave>(deviceResources);
}

void WaveManager::Initialize(HWND hwnd, int width, int height)
{
	//”g‚Ì‰Šú‰»
	m_wave->Initialize(hwnd, width, height);

	//‰Šú‰»‚Éƒ‚[ƒh‚ğ”½‰f
	if(ms_isCubeMode)
	{
		m_wave->ToggleDotMode();
	}
}

void WaveManager::Update(float deltaTime)
{
	

	//”g‚ÌXV
	m_wave->Update(deltaTime);
	
}

void WaveManager::Render(
	ID3D11DeviceContext* context,
	const DirectX::SimpleMath::Matrix& view,
	const DirectX::SimpleMath::Matrix& proj)
{
	
	m_wave->Render(context, view, proj);
	
}

void WaveManager::ToggleMode()
{
	if (m_wave)
	{
		//ƒ‚[ƒh‚ğØ‚è‘Ö‚¦‚éƒtƒ‰ƒO‚ğ”½“]
		ms_isCubeMode = !ms_isCubeMode;

		//ƒ‚[ƒh‚ğØ‚è‘Ö‚¦‚é
		m_wave->ToggleDotMode();
	}
}

DirectX::SimpleMath::Vector2 WaveManager::GetCurrentWaveAngle(float x, float z) const
{
	//”g‚ÌŠp“x‚ğæ“¾
	return m_wave->GetWaveAngle(x, z);
	
}

float WaveManager::GetCurrentHeight(float x, float z) const
{
	//”g‚Ì‚‚³‚ğæ“¾
	return m_wave->GetHeight(x, z);

}
