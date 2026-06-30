
/**
 * @file   PlayerCamera.h
 * @brief  プレイヤーカメラの動きの管理を行うクラス
 * @author 國田知睦
 * @date   2026/06/19
 */

#pragma once

#include <SimpleMath.h>
#include <Mouse.h>
#include <Keyboard.h>

/// <summary>
/// ゲームプレイ中のカメラクラス
/// </summary>
class PlayerCamera 
{
private:

	//マウス操作によるカメラの回転速度
	static constexpr float CAMERA_ROT_SPEED = 0.003f;

	//通常時のカメラとプレイヤーの距離
	static constexpr float DEFAULT_CAMERA_DISTANCE =10.0f;
	
	//ダッシュした際にカメラがプレイヤーに追従する速度
	static constexpr float CAMERA_LERP_SPEED = 0.01f;

	//カメラの基本の高さのオフセット
	static constexpr float CAMERA_OFFSET_Y = 5.0f;
	//通常時のカメラが向く目標地点
	static constexpr float NORMAL_TARGET_Y = 5.5f;

	//ダッシュ時のプレイヤーとカメラの距離
	static constexpr float DASH_DISTANCE = 5.0f;
	//ダッシュ時のカメラが向く目標地点
	static constexpr float DASH_TARGET_Y = 6.5f;

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

private:

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

	//目標のカメラの高さ
	float goalTargetY;

	//目標のカメラとの距離
	float goalDist;

};


