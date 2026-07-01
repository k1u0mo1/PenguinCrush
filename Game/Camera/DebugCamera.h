
/**
 * @file   DebugCamera.h
 * @brief  確認用デバッグカメラの動きの管理を行うクラス
 * @author 國田知睦
 * @date   2026/07/01
 */

#pragma once
#include <SimpleMath.h>
#include <Mouse.h>
#include <Keyboard.h>

/// <summary>
/// ゲームプレイ中のデバッグ用カメラクラス
/// </summary>
class DebugCamera
{
private:

	//カメラの間隔
	static constexpr float DEFAULT_CAMERA_DISTANCE = 5.0f;
	//ホイールのスクロール感度
	static constexpr float SCROLL_SENSITIVITY = 0.005f;
	//カメラの最低距離の制限
	static constexpr float DISTANCE_MIN = 1.0f;
	//カメラの最大距離の制限
	static constexpr float DISTANCE_MAX = 50.0f;
	//キーボード操作時の移動の速さ
	static constexpr float MOVE_SPEED = 0.5f;
	//方向ベクトルがほぼゼロかどうか判定用
	static constexpr float ZERO_THRESHOLD = 0.001f;
	//注視点からの高さのオフセット
	static constexpr float CAMERA_OFFSET_Y = 2.0;

private:

	/// <summary>
	/// 行列の生成
	/// </summary>
	/// <param name="x">X軸</param>
	/// <param name="y">Y軸</param>
	void Motion(int x, int y);

public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="windowWidth">ウインドウサイズ（幅）</param>
	/// <param name="windowHeight">ウインドウサイズ（高さ）</param>
	DebugCamera(int windowWidth, int windowHeight);

	/// <summary>
	/// デバッグカメラの更新
	/// </summary>
	/// <param name="mouse">マウスオブジェクトへのポインタ</param>
	void Update();

	/// <summary>
	/// デバッグカメラのビュー行列の取得関数
	/// </summary>
	/// <returns>ビュー行列</returns>
	DirectX::SimpleMath::Matrix GetCameraMatrix()const;

	/// <summary>
	/// デバッグカメラの位置の取得関数
	/// </summary>
	/// <returns>視点の位置</returns>
	DirectX::SimpleMath::Vector3 GetEyePosition()const;

	/// <summary>
	/// デバッグカメラの注視点の取得関数
	/// </summary>
	/// <returns>注視点の位置</returns>
	DirectX::SimpleMath::Vector3 GetTargetPosition()const;

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
	void GetWindowSize(int& windowWidth, int& windowHeight) const;

	/// <summary>
	/// 視点
	/// </summary>
	/// <param name="playerPos">プレイヤーの座標</param>
	void UpdateTarget(const DirectX::SimpleMath::Vector3& playerPos);

	/// <summary>
	/// 切り替えた時にCamera情報をもらっておく
	/// </summary>
	/// <param name="eye">視点</param>
	/// <param name="target">注視点</param>
	void SetFromOtherCamera(
		const DirectX::SimpleMath::Vector3& eye,
		const DirectX::SimpleMath::Vector3& target);

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
	float m_distance;
};