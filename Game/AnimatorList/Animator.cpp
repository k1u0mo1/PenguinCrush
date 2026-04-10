#include "pch.h"
#include "Animator.h"

Animator::Animator()
	: m_currentAnimation()
	, m_currentTime()
	, m_isLooping()
{
}

void Animator::Update(float elapsedTime)
{
	if (m_currentAnimation.empty())return;

	//再生時間を決める
	m_currentTime += elapsedTime;

	//最終的な行列を計算
	CalculateBoneMatrices(0, DirectX::SimpleMath::Matrix::Identity);
}

void Animator::Play(const std::string& /*animName*/, bool /*loop*/)
{
	return;
}

void Animator::CalculateBoneMatrices(int /*boneId*/, const DirectX::SimpleMath::Matrix& /*parentTransform*/)
{
	return;
}
