
// シーンへ渡すユーザーリソースクラス（サンプル）
#pragma once

#include "Common/StepTimer.h"
#include "Common/DeviceResources.h"
#include "Library/DebugFont.h"
#include "Library/InputManager.h"

//フェード
#include "Common/TransitionMask.h"

// 各シーンに渡す共通リソース
class UserResources
{
private:

	// ステップタイマー
	DX::StepTimer* m_timer;

	// デバイスリソース
	DX::DeviceResources* m_deviceResources;

	// キーボードステートトラッカー
	DirectX::Keyboard::KeyboardStateTracker* m_keyboardTracker;

	// マウスステートトラッカー
	DirectX::Mouse::ButtonStateTracker* m_mouseTracker;

	// デバッグ用文字列表示へのポインタ
	DebugFont* m_debugFont;

	// 共通ステート
	DirectX::CommonStates* m_states;

	//入力関連 ポインタ
	InputManager* m_inputManager;

	//------------------------------------------------
	//画面遷移
	//------------------------------------------------
	TransitionMask* m_transitionMask;
	

public:

	// コンストラクタ
	UserResources()
		: m_timer(nullptr)
		, m_deviceResources(nullptr)
		, m_keyboardTracker(nullptr)
		, m_mouseTracker(nullptr)
		, m_debugFont(nullptr)
		, m_states(nullptr)
		, m_transitionMask(nullptr)
		, m_inputManager(nullptr)
	{
	}

	
	// ステップタイマー	//////////////////////////////////////
	

	// ステップタイマーを設定する関数
	void SetStepTimerStates(DX::StepTimer* timer) { m_timer = timer; }

	// ステップタイマーを取得する関数
	DX::StepTimer* GetStepTimer() { return m_timer; }

	
	// デバイスリソース	///////////////////////////////////////
	

	// デバイスリソースを設定する関数
	void SetDeviceResources(DX::DeviceResources* deviceResources) { m_deviceResources = deviceResources; }

	// デバイスリソースを取得する関数
	DX::DeviceResources* GetDeviceResources() { return m_deviceResources; }

	
	// キーボードステートトラッカー	///////////////////////////////////////
	

	// キーボードステートトラッカーを設定する関数
	void SetKeyboardStateTracker(DirectX::Keyboard::KeyboardStateTracker* tracker) { m_keyboardTracker = tracker; }

	// キーボードステートトラッカーを取得する関数
	DirectX::Keyboard::KeyboardStateTracker* GetKeyboardStateTracker() { return m_keyboardTracker; }

	
	// マウスステートトラッカー	////////////////////////////////////
	

	// マウスステートトラッカーを設定する関数
	void SetMouseStateTracker(DirectX::Mouse::ButtonStateTracker* tracker) { m_mouseTracker = tracker; }

	// マウスステートトラッカーを取得する関数
	DirectX::Mouse::ButtonStateTracker* GetMouseStateTracker() { return m_mouseTracker; }

	
	// 共通ステート	//////////////////////////////////////////
	

	// 共通ステートを設定する関数
	void SetCommonStates(DirectX::CommonStates* states) { m_states = states; }

	// 共通ステートを取得する関数
	DirectX::CommonStates* GetCommonStates() { return m_states; }

	// デバッグフォント	///////////////////////////////////////////

	// デバッグフォントを設定する関数
	void SetDebugFont(DebugFont* debugFont) { m_debugFont = debugFont; }

	// デバッグフォントを取得する関数
	DebugFont* GetDebugFont() { return m_debugFont; }

	//入力関連////////////////////////////////////////////////////////////

	//入力関連を設定する関連
	void SetInputManager(InputManager* inputManager) { m_inputManager = inputManager; }

	//入力関連を取得する関数
	InputManager* GetInputManager() { return m_inputManager; }

	//------------------------------------------ //
	//画面遷移マスク	
	//------------------------------------------ //

	//画面遷移マスクを設定する関数
	void SetTransitionMask(TransitionMask* transitionMask)
	{
		m_transitionMask = transitionMask;

	}

	//画面遷移マスクを取得する関数
	TransitionMask* GetTransitionMask() { return m_transitionMask; }


};

