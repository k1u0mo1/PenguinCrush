
/**
 * @file   HitStop.h
 * @brief  プレイヤーと敵のヒットストップを管理するクラス
 * @author 國田知睦
 * @date   2026/07/01
 */

#pragma once
#include "pch.h"
#include "Game/Common/DeviceResources.h"

/// <summary>
/// 攻撃後の共通ヒットストップを管理するクラス
/// </summary>
class HitStop
{
public:

    /// <summary>
    /// コンストラクタ
    /// </summary>
    HitStop();

    /// <summary>
	/// ヒットストップの開始
    /// </summary>
    /// <param name="duration">ヒットストップの持続時間</param>
    void StartHitStop(float duration) { m_timer = duration; };

    /// <summary>
    /// ヒットストップの更新
    /// </summary>
    /// <param name="deltaTime">前フレームからの経過時間</param>
    void HitStopUpdate(float deltaTime);
    
	/// <summary>
	/// 現在ヒットストップ中か
	/// </summary>
	/// <returns></returns>
	bool IsHitStopActive() const { return m_timer > 0.0f; }

private:

	//ヒットストップの持続時間
    float m_timer;

};