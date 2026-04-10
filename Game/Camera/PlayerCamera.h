//--------------------------------------------------------------------------------------
//PlayerCamera.h
//--------------------------------------------------------------------------------------
#pragma once


#include <SimpleMath.h>
#include <Mouse.h>
#include <Keyboard.h>


/// <summary>
/// ゲームプレイ中のカメラクラス
/// </summary>
class PlayerCamera 
{
	// カメラの距離
	static const float DEFAULT_CAMERA_DISTANCE;

	// 横回転
	float m_yAngle, m_yTmp;

	// 縦回転
	float m_xAngle, m_xTmp;

	// ドラッグされた座標
	int m_x, m_y;

	float m_sx, m_sy;

	// 生成されたビュー行列
	DirectX::SimpleMath::Matrix m_view;

	// スクロールフォイール値
	int m_scrollWheelValue;

	// 視点
	DirectX::SimpleMath::Vector3 m_eye;

	// 注視点
	DirectX::SimpleMath::Vector3 m_target;

	// マウストラッカー
	DirectX::Mouse::ButtonStateTracker m_tracker;

	// スクリーンサイズ
	int m_screenW, m_screenH;

	//現在の注視点のY軸
	float m_targetY;

	//現在のカメラの距離
	float m_currentDist;

private:

	/// <summary>
	/// 行列の生成
	/// </summary>
	/// <param name="x">X軸</param>
	/// <param name="y">Y軸</param>
	void Motion(int x, int y);

	int m_prevMouseX = 0;
	int m_prevMouseY = 0;

	HWND m_hwnd = nullptr;

public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="windowWidth">ウインドウサイズ（幅）</param>
	/// <param name="windowHeight">ウインドウサイズ（高さ）</param>
	PlayerCamera(int windowWidth, int windowHeight, HWND hwnd);

	/// <summary>
	/// プレイヤーカメラの更新
	/// </summary>
	/// <param name="playerPos">プレイヤーの座標</param>
	/// <param name="mouseState">マウスの状態</param>
	/// <param name="isPlayerDashing">プレイヤーがダッシュ中かどうか</param>
	void Update(
		const DirectX::SimpleMath::Vector3& playerPos, 
		const DirectX::Mouse::State& mouseState,
		bool isPlayerDashing);

	/// <summary>
	/// プレイヤーカメラのビュー行列の取得関数
	/// </summary>
	/// <returns>ビュー行列</returns>
	DirectX::SimpleMath::Matrix GetCameraMatrix() const ;

	/// <summary>
	/// プレイヤーカメラの位置の取得関数
	/// </summary>
	/// <returns>視点の位置</returns>
	DirectX::SimpleMath::Vector3 GetEyePosition() const;

	/// <summary>
	/// プレイヤーカメラの注視点の取得関数
	/// </summary>
	/// <returns>注視点の位置</returns>
	DirectX::SimpleMath::Vector3 GetTargetPosition() const;

	/// <summary>
	/// 画面サイズの設定関数
	/// </summary>
	/// <param name="windowWidth">ウインドウサイズ（幅）</param>
	/// <param name="windowHeight">ウインドウサイズ（高さ）</param>
	void SetWindowSize(int windowWidth, int windowHeight);

	/// <summary>
	/// 画面サイズの取得関数
	/// </summary>
	/// <param name="windowWidth">取得した幅を格納する変数</param>
	/// <param name="windowHeight">取得した高さを格納する変数</param>
	void GetWindowSize(
		int& windowWidth,
		int& windowHeight
	) const;

	/// <summary>
	/// カーソル非表示
	/// </summary>
	void HideCursor();

	/// <summary>
	/// カーソル表示
	/// </summary>
	void ShowCursor();   



};


