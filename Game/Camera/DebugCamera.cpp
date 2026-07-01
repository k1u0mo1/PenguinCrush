
/**
 * @file   DebugCamera.cpp
 * @brief  確認用デバッグカメラの動きの管理を行うクラス
 * @author 國田知睦
 * @date   2026/07/01
 */

#include "pch.h"
#include "DebugCamera.h"
#include "Mouse.h"
#include <Library/InputManager.h>

//--------------------------------------------------------------------------------------
// コンストラクタ
//--------------------------------------------------------------------------------------

DebugCamera::DebugCamera(int windowWidth, int windowHeight)
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
	m_distance(DEFAULT_CAMERA_DISTANCE)
{
	SetWindowSize(windowWidth, windowHeight);

	// マウスのフォイール値をリセット
	DirectX::Mouse::Get().ResetScrollWheelValue();
}

//--------------------------------------------------------------------------------------
// 更新
//--------------------------------------------------------------------------------------

void DebugCamera::Update()
{
	DirectX::Mouse::State state = DirectX::Mouse::Get().GetState();

	// 前フレームからのスクロール変化量を取得し、距離に加算 
	int scrollDelta = state.scrollWheelValue - m_scrollWheelValue;
	// 今回の値を保存
	m_scrollWheelValue = state.scrollWheelValue; 

	// スクロールホイール値に応じて距離を更新
	m_distance -= scrollDelta * SCROLL_SENSITIVITY;

	// 距離に制限を設ける
	m_distance = std::max(DISTANCE_MIN, m_distance); 
	m_distance = std::min(DISTANCE_MAX, m_distance); 

	m_tracker.Update(state);

	if (m_tracker.leftButton == DirectX::Mouse::ButtonStateTracker::PRESSED)
	{
		m_x = state.x;
		m_y = state.y;
	}
	else if (m_tracker.leftButton == DirectX::Mouse::ButtonStateTracker::RELEASED)
	{
		m_xAngle = m_xTmp;
		m_yAngle = m_yTmp;
	}

	if (state.leftButton)
	{
		Motion(state.x, state.y);
	}

	//-----------------------------------------------------
	//デバッグに切り替えた際にキーボード操作による自由移動
	//-----------------------------------------------------
	DirectX::Keyboard::State kb = DirectX::Keyboard::Get().GetState();

	//現在のカメラの向いている水平方向と左右の方向を計算
	DirectX::SimpleMath::Vector3 forward = m_target - m_eye;
	//水平に移動させるためにY成分を消す
	forward.y = 0.0f;

	if (forward.LengthSquared() > ZERO_THRESHOLD)
	{
		forward.Normalize();
	}
	else 
	{
		forward = DirectX::SimpleMath::Vector3::Transform(DirectX::SimpleMath::Vector3::Forward, DirectX::SimpleMath::Matrix::CreateRotationY(m_yTmp));
	}

	DirectX::SimpleMath::Vector3 right = DirectX::SimpleMath::Vector3::Up.Cross(forward);
	right.Normalize();

	DirectX::SimpleMath::Vector3 worldUp = DirectX::SimpleMath::Vector3::Up;
	DirectX::SimpleMath::Vector3 move = DirectX::SimpleMath::Vector3::Zero;

	// WASDキーで前後左右に移動
	if (kb.W) move += forward;
	if (kb.S) move -= forward;
	if (kb.D) move -= right;
	if (kb.A) move += right;

	// E・Qキーで上下に昇降
	if (kb.E) move += worldUp;
	if (kb.Q) move -= worldUp;

	// 入力があった場合、注視点（m_target）をスライドさせる
	if (move != DirectX::SimpleMath::Vector3::Zero)
	{
		move.Normalize();
		m_target += move * MOVE_SPEED;
	}
	//-----------------------------------------------------

	// 回転
	DirectX::SimpleMath::Matrix rotY = DirectX::SimpleMath::Matrix::CreateRotationY(m_yTmp);
	DirectX::SimpleMath::Matrix rotX = DirectX::SimpleMath::Matrix::CreateRotationX(m_xTmp);
	DirectX::SimpleMath::Matrix rt = rotY * rotX;

	// プレイヤーを注視
	DirectX::SimpleMath::Vector3 up = DirectX::SimpleMath::Vector3::Transform(DirectX::SimpleMath::Vector3::UnitY, rt.Invert());
	
	DirectX::SimpleMath::Vector3 offset(0.0f, CAMERA_OFFSET_Y, m_distance);
	offset = DirectX::SimpleMath::Vector3::Transform(offset, rt);

	m_eye = m_target + offset;
	m_view = DirectX::SimpleMath::Matrix::CreateLookAt(m_eye, m_target, up);
}

//--------------------------------------------------------------------------------------
// 行列の生成
//--------------------------------------------------------------------------------------

void DebugCamera::Motion(int x, int y)
{
	// マウスポインタの位置のドラッグ開始位置からの変位 (相対値)
	float dx = (x - m_x) * m_sx;
	float dy = (y - m_y) * m_sy;

	if (dx != 0.0f || dy != 0.0f)
	{
		// Ｙ軸の回転
		float yAngle = dx * DirectX::XM_PI;
		// Ｘ軸の回転
		float xAngle = dy * DirectX::XM_PI;

		m_xTmp = m_xAngle + xAngle;
		m_yTmp = m_yAngle + yAngle;
	}
}

//--------------------------------------------------------------------------------------
// デバッグカメラのビュー行列の取得関数
//--------------------------------------------------------------------------------------

DirectX::SimpleMath::Matrix DebugCamera::GetCameraMatrix()const
{
	return m_view;
}

//--------------------------------------------------------------------------------------
// デバッグカメラの位置の取得関数
//--------------------------------------------------------------------------------------

DirectX::SimpleMath::Vector3 DebugCamera::GetEyePosition()const
{
	return m_eye;
}

//--------------------------------------------------------------------------------------
// デバッグカメラの注視点の取得関数
//--------------------------------------------------------------------------------------

DirectX::SimpleMath::Vector3 DebugCamera::GetTargetPosition()const
{
	return m_target;
}

//--------------------------------------------------------------------------------------
// 画面サイズの設定関数
//--------------------------------------------------------------------------------------

void DebugCamera::SetWindowSize(int windowWidth, int windowHeight)
{
	// 画面サイズに対する相対的なスケールに調整
	m_sx = 1.0f / float(windowWidth);
	m_sy = 1.0f / float(windowHeight);
}

//--------------------------------------------------------------------------------------
// 画面サイズの取得関数
//--------------------------------------------------------------------------------------

void DebugCamera::GetWindowSize(int& windowWidth, int& windowHeight) const
{
	windowWidth = m_screenW;
	windowHeight = m_screenH;
}

//--------------------------------------------------------------------------------------
// 視点
//--------------------------------------------------------------------------------------

void DebugCamera::UpdateTarget(const DirectX::SimpleMath::Vector3& playerPos)
{
	m_target = playerPos;
}

//--------------------------------------------------------------------------------------
// 切り替えた時にCamera情報をもらっておく
//--------------------------------------------------------------------------------------

void DebugCamera::SetFromOtherCamera(const DirectX::SimpleMath::Vector3& eye, const DirectX::SimpleMath::Vector3& target)
{
	m_eye = eye;
	m_target = target;

	// 角度計算（カメラ方向から回転角へ）
	DirectX::SimpleMath::Vector3 dir = target - eye;
	m_distance = dir.Length();
	dir.Normalize();

	m_yTmp = atan2f(dir.x, dir.z);     // Y 回転
	m_xTmp = -asinf(dir.y);            // X 回転

	m_xAngle = m_xTmp;
	m_yAngle = m_yTmp;
}
