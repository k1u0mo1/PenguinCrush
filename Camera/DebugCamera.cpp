#include "pch.h"
#include "DebugCamera.h"
#include "Mouse.h"

using namespace DirectX;

const float DebugCamera::DEFAULT_CAMERA_DISTANCE = 5.0f;

//--------------------------------------------------------------------------------------
// コンストラクタ
//--------------------------------------------------------------------------------------
DebugCamera::DebugCamera(int windowWidth, int windowHeight)
	: m_yAngle(0.0f)
	, m_yTmp(0.0f)
	, m_xAngle(0.0f)
	, m_xTmp(0.0f)
	, m_x(0)
	, m_y(0)
	, m_scrollWheelValue(0)
	, m_screenW(windowWidth)
	, m_screenH(windowHeight)
{
	SetWindowSize(windowWidth, windowHeight);

	// マウスのフォイール値をリセット
	Mouse::Get().ResetScrollWheelValue();
}

//--------------------------------------------------------------------------------------
// 更新
//--------------------------------------------------------------------------------------
void DebugCamera::Update()
{
	Mouse::State state = Mouse::Get().GetState();

	// 前フレームからのスクロール変化量を取得し、距離に加算 
	int scrollDelta = state.scrollWheelValue - m_scrollWheelValue;
	m_scrollWheelValue = state.scrollWheelValue; // 今回の値を保存

	// スクロールホイール値に応じて距離を更新
	const float SCROLL_SENSITIVITY = 0.005f;
	m_distance -= scrollDelta * SCROLL_SENSITIVITY;

	// 距離に制限を設ける
	m_distance = std::max(1.0f, m_distance); // 最低距離 1.0f
	m_distance = std::min(50.0f, m_distance); // 最大距離 50.0f



	m_tracker.Update(state);

	if (m_tracker.leftButton == Mouse::ButtonStateTracker::PRESSED)
	{
		m_x = state.x;
		m_y = state.y;
	}
	else if (m_tracker.leftButton == Mouse::ButtonStateTracker::RELEASED)
	{
		m_xAngle = m_xTmp;
		m_yAngle = m_yTmp;
	}

	if (state.leftButton)
	{
		Motion(state.x, state.y);
	}

	// 回転
	Matrix rotY = Matrix::CreateRotationY(m_yTmp);
	Matrix rotX = Matrix::CreateRotationX(m_xTmp);
	Matrix rt = rotY * rotX;

	// プレイヤーを注視
	Vector3 up = Vector3::Transform(Vector3::UnitY, rt.Invert());

	// プレイヤーカメラと似た距離扱い
	//float distance = DEFAULT_CAMERA_DISTANCE - state.scrollWheelValue * 0.01f;

	Vector3 offset(0.0f, 2.0f, m_distance);
	offset = Vector3::Transform(offset, rt);

	m_eye = m_target + offset;

	m_view = Matrix::CreateLookAt(m_eye, m_target, up);
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
		float yAngle = dx * XM_PI;
		// Ｘ軸の回転
		float xAngle = dy * XM_PI;

		m_xTmp = m_xAngle + xAngle;
		m_yTmp = m_yAngle + yAngle;
	}
}

DirectX::SimpleMath::Matrix DebugCamera::GetCameraMatrix()const
{
	return m_view;
}

DirectX::SimpleMath::Vector3 DebugCamera::GetEyePosition()const
{
	return m_eye;
}

DirectX::SimpleMath::Vector3 DebugCamera::GetTargetPosition()const
{
	return m_target;
}

void DebugCamera::SetWindowSize(int windowWidth, int windowHeight)
{
	// 画面サイズに対する相対的なスケールに調整
	m_sx = 1.0f / float(windowWidth);
	m_sy = 1.0f / float(windowHeight);
}

void DebugCamera::GetWindowSize(int& windowWidth, int& windowHeight) const
{
	windowWidth = m_screenW;
	windowHeight = m_screenH;
}

void DebugCamera::UpdateTarget(const Vector3& playerPos)
{
	m_target = playerPos;
}

void DebugCamera::SetFromOtherCamera(const DirectX::SimpleMath::Vector3& eye, const DirectX::SimpleMath::Vector3& target)
{
	m_eye = eye;
	m_target = target;

	// 角度計算（カメラ方向から回転角へ）
	Vector3 dir = target - eye;
	dir.Normalize();

	m_yTmp = atan2f(dir.x, dir.z);     // Y 回転
	m_xTmp = -asinf(dir.y);            // X 回転

	m_xAngle = m_xTmp;
	m_yAngle = m_yTmp;
}
