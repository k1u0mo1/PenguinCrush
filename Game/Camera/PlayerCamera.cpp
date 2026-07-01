
/**
 * @file   PlayerCamera.cpp
 * @brief  プレイヤーカメラの動きの管理を行うクラス
 * @author 國田知睦
 * @date   2026/07/01
 */

#include "pch.h"
#include "PlayerCamera.h"
#include "Mouse.h"

//--------------------------------------------------------------------------------------
// コンストラクタ
//--------------------------------------------------------------------------------------

PlayerCamera::PlayerCamera(int windowWidth, int windowHeight, HWND hwnd)
	: 
	m_yAngle(0.0f),
	m_yTmp(0.0f),
	m_xAngle(0.0f),
	m_xTmp(0.0f),
	m_x(0),
	m_y(0),
	m_scrollWheelValue(0),
	m_screenW(windowWidth),
	m_screenH(windowHeight),
	m_hwnd(hwnd),
	m_targetY(NORMAL_TARGET_Y),
	m_currentDist(DEFAULT_CAMERA_DISTANCE)
{
	SetWindowSize(windowWidth, windowHeight);

	// マウスのフォイール値をリセット
	DirectX::Mouse::Get().ResetScrollWheelValue();

	HideCursor();
}

//--------------------------------------------------------------------------------------
// プレイヤーカメラの更新
//--------------------------------------------------------------------------------------

void PlayerCamera::Update(
	const DirectX::SimpleMath::Vector3& playerPos,
	const DirectX::Mouse::State& mouseState,
	bool isPlayerDashing)
{
	if (m_hwnd)
	{
		POINT pt = { m_screenW / 2, m_screenH / 2 };

		// クライアント座標（ウィンドウ内の座標）をスクリーン座標（画面全体の座標）に変換
		ClientToScreen(m_hwnd, &pt);
		// マウスカーソルを画面中央に移動
		SetCursorPos(pt.x, pt.y);
		// 次フレームの基準点 m_prevMouseX/Y をウィンドウ中央に設定
		m_prevMouseX = m_screenW / 2;
		m_prevMouseY = m_screenH / 2;
	}

	// 前フレームとの差分（X軸のみ）
	float dx = float(mouseState.x - m_prevMouseX);
	// 次フレーム用に保存
	m_prevMouseX = mouseState.x;

	// 左右回転のみ
	m_yTmp -= dx * CAMERA_ROT_SPEED;

	//-------------------------------------------------
	//カメラの移動処理
	//-------------------------------------------------

	//切り替え注視点
	//ダッシュ中
	if (isPlayerDashing)
	{
		//カメラの高さ
		goalTargetY = DASH_TARGET_Y;
		//プレイヤーとカメラの距離
		goalDist = DASH_DISTANCE;
	}
	//通常
	else
	{		
		//カメラの高さ
		goalTargetY = NORMAL_TARGET_Y;
		//プレイヤーとカメラの距離
		goalDist = DEFAULT_CAMERA_DISTANCE;
	}

	//新しい値＝現在の値 ＋（ 　　目標までの距離　　）× 移動する割合
	m_targetY = m_targetY + (goalTargetY - m_targetY) * CAMERA_LERP_SPEED;
	//新しい値　　＝　現在の値 　＋（ 　　目標までの距離　　）× 移動する割合
	m_currentDist = m_currentDist + (goalDist - m_currentDist) * CAMERA_LERP_SPEED;
	// 注視点
	m_target = playerPos + DirectX::SimpleMath::Vector3(0.0f, m_targetY, 0.0f);
	// カメラ後方オフセット
	DirectX::SimpleMath::Vector3 offset(0.0f, CAMERA_OFFSET_Y, m_currentDist);
	// Y軸回転のみ
	DirectX::SimpleMath::Matrix rotY = DirectX::SimpleMath::Matrix::CreateRotationY(m_yTmp);
	offset = DirectX::SimpleMath::Vector3::Transform(offset, rotY);

	// カメラ位置
	m_eye = m_target + offset;
	// ビュー行列
	m_view = DirectX::SimpleMath::Matrix::CreateLookAt(m_eye, m_target, DirectX::SimpleMath::Vector3::UnitY);
}

//--------------------------------------------------------------------------------------
// 行列の生成
//--------------------------------------------------------------------------------------

void PlayerCamera::Motion(int x, int y)
{
	// マウスポインタの位置のドラッグ開始位置からの変位 (相対値)
	float dx = (x - m_x) * m_sx;
	float dy = (y - m_y) * m_sy;

	if (dx != 0.0f || dy != 0.0f)
	{
		// Ｙ軸の回転
		float yAngle = dx * DirectX::XM_PI;
		m_yTmp = m_yAngle + yAngle;
	}
}

//--------------------------------------------------------------------------------------
// プレイヤーカメラのビュー行列の取得関数
//--------------------------------------------------------------------------------------

DirectX::SimpleMath::Matrix PlayerCamera::GetCameraMatrix()const
{
	return m_view;
}

//--------------------------------------------------------------------------------------
// プレイヤーカメラの位置の取得関数
//--------------------------------------------------------------------------------------

DirectX::SimpleMath::Vector3 PlayerCamera::GetEyePosition()const
{
	return m_eye;
}

//--------------------------------------------------------------------------------------
// プレイヤーカメラの注視点の取得関数
//--------------------------------------------------------------------------------------

DirectX::SimpleMath::Vector3 PlayerCamera::GetTargetPosition()const
{
	return m_target;
}

//--------------------------------------------------------------------------------------
// 画面サイズの設定関数
//--------------------------------------------------------------------------------------

void PlayerCamera::SetWindowSize(int windowWidth, int windowHeight)
{
	// 画面サイズに対する相対的なスケールに調整
	m_sx = 1.0f / float(windowWidth);
	m_sy = 1.0f / float(windowHeight);
}

//--------------------------------------------------------------------------------------
// 画面サイズの取得関数
//--------------------------------------------------------------------------------------

void PlayerCamera::GetWindowSize(int & windowWidth, int & windowHeight) const
{
	windowWidth = m_screenW;
	windowHeight = m_screenH;
}

//--------------------------------------------------------------------------------------
// カーソル非表示
//--------------------------------------------------------------------------------------

void PlayerCamera::HideCursor()
{
	::ShowCursor(FALSE);
}

//--------------------------------------------------------------------------------------
// カーソル表示
//--------------------------------------------------------------------------------------

void PlayerCamera::ShowCursor()
{
	::ShowCursor(TRUE);
}
