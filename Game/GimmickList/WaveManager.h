
#pragma once
#include "pch.h"
#include <Game/Common/DeviceResources.h>

#include "Game/GimmickList/Wave.h"

#include <memory>

/// <summary>
/// 波の生成と描画を管理するクラス
/// 各シーンでインスタンス化されるが、モードは全シーンで共有
/// </summary>
class WaveManager
{
public:

	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="deviceResources">デバイスリソース</param>
	WaveManager(DX::DeviceResources* deviceResources);

	/// <summary>
	/// 波の初期化
	/// </summary>
	/// <param name="hwnd">ウィンドウハンドル</param>
	/// <param name="width">画面の幅</param>
	/// <param name="height">画面の高さ</param>
	void Initialize(HWND hwnd, int width, int height);

	/// <summary>
	/// 更新
	/// </summary>
	/// <param name="deltaTime">前フレームからの経過時間</param>
	void Update(float deltaTime);

	/// <summary>
	/// 描画
	/// </summary>
	/// <param name="context">デバイスコンテキスト</param>
	/// <param name="view">ビュー行列</param>
	/// <param name="proj">プロジェクション行列</param>
	void Render(
		ID3D11DeviceContext* context, 
		const DirectX::SimpleMath::Matrix& view,
		const DirectX::SimpleMath::Matrix& proj
	);

	/// <summary>
	/// 波のモードを切り替える　全シーンに影響する
	/// </summary>
	void ToggleMode();

	/// <summary>
	/// 現在の波のモードを取得　trueなら立方体、falseなら通常の波
	/// </summary>
	/// <returns>trueなら立方体、falseなら通常の波</returns>
	static bool IsCubeMode() { return ms_isCubeMode; }

	/// <summary>
	/// 波のモードを直接設定する　全シーンに影響する
	/// </summary>
	/// <param name="isCubeMode">trueなら立方体、falseなら通常の波</param>
	static void SetCubeMode(bool isCubeMode) { ms_isCubeMode = isCubeMode; }

	/// <summary>
	/// 現在の波の角度を取得
	/// </summary>
	/// <param name="x">X座標</param>
	/// <param name="z">Z座標</param>
	/// <returns>波の角度</returns>
	DirectX::SimpleMath::Vector2 GetCurrentWaveAngle(float x, float z)const;

	/// <summary>
	/// 現在の波の高さを取得		
	/// </summary>
	/// <param name="x">X座標</param>
	/// <param name="z">Z座標</param>
	/// <returns>波の高さ</returns>
	float GetCurrentHeight(float x, float z)const;


private:

	std::unique_ptr<Wave> m_wave;

	//波のモード　trueなら立方体、falseなら通常の波
	static bool ms_isCubeMode;
};