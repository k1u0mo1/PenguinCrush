//--------------------------------------------------------------------------------------
// PlayerCamera.cpp
//--------------------------------------------------------------------------------------
#include "pch.h"
#include "PlayerCamera.h"
#include "Mouse.h"

using namespace DirectX;

const float PlayerCamera::DEFAULT_CAMERA_DISTANCE = 5.0f;

//--------------------------------------------------------------------------------------
// コンストラクタ
//--------------------------------------------------------------------------------------

PlayerCamera::PlayerCamera(int windowWidth, int windowHeight, HWND hwnd)
	: m_yAngle(0.0f)
	, m_yTmp(0.0f)
	, m_xAngle(0.0f)
	, m_xTmp(0.0f)
	, m_x(0)
	, m_y(0)
	, m_scrollWheelValue(0)
	, m_screenW(windowWidth)
	, m_screenH(windowHeight)
	, m_hwnd(hwnd)
	, m_targetY(2.5f)
	, m_currentDist(DEFAULT_CAMERA_DISTANCE)
{
	SetWindowSize(windowWidth, windowHeight);

	// マウスのフォイール値をリセット
	Mouse::Get().ResetScrollWheelValue();

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
	const float rotSpeed = 0.003f;
	m_yTmp -= dx * rotSpeed;

	//-------------------------------------------------
	//カメラの移動処理
	//-------------------------------------------------
	
	//目標のカメラの高さ
	float goalTargetY = 0.0f;
	//目標のカメラとの距離
	float goalDist = 0.0f;

	//切り替え注視点
	if (isPlayerDashing)
	{
		//ダッシュ中
		//m_target = playerPos + SimpleMath::Vector3(0.0f, m_targetY, 0.0f);
		
		goalTargetY = 1.5f;

		goalDist = 4.5f;
	}
	else
	{		
		//通常
		//m_target = playerPos + SimpleMath::Vector3(0.0f, m_targetY, 0.0f);

		goalTargetY = 2.5f;

		goalDist = DEFAULT_CAMERA_DISTANCE;
	}

	//現在の値を目標の位置に近づける
	float lerpSpeed = 0.05f;

	//新しい値＝現在の値 ＋（ 　　目標までの距離　　）× 移動する割合
	m_targetY = m_targetY + (goalTargetY - m_targetY) * lerpSpeed;

	//新しい値　　＝　現在の値 　＋（ 　　目標までの距離　　）× 移動する割合
	m_currentDist = m_currentDist + (goalDist - m_currentDist) * lerpSpeed;

	// 注視点
	m_target = playerPos + SimpleMath::Vector3(0.0f, m_targetY, 0.0f);

	// カメラ後方オフセット
	SimpleMath::Vector3 offset(0.0f, 2.0f, m_currentDist);

	// Y軸回転のみ
	SimpleMath::Matrix rotY = SimpleMath::Matrix::CreateRotationY(m_yTmp);
	offset = DirectX::SimpleMath::Vector3::Transform(offset, rotY);

	// カメラ位置
	m_eye = m_target + offset;

	// ビュー行列
	m_view = SimpleMath::Matrix::CreateLookAt(m_eye, m_target, SimpleMath::Vector3::UnitY);
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
		float yAngle = dx * XM_PI;
		// Ｘ軸の回転
		//float xAngle = dy * XM_PI;

		//m_xTmp = m_xAngle + xAngle;
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
