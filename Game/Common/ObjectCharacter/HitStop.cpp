
/**
 * @file   HitStop.cpp
 * @brief  プレイヤーと敵のヒットストップを管理するクラス
 * @author 國田知睦
 * @date   2026/06/17
 */


#include "pch.h"
#include "HitStop.h"

HitStop::HitStop()
	: m_timer(0.0f)
{
}

void HitStop::HitStopUpdate(float deltaTime)
{

	if(m_timer > 0.0f)
	{
		m_timer -= deltaTime;
		if (m_timer < 0.0f)
		{
			m_timer = 0.0f;
		}
	}

}
