//
// Game.h
//

#pragma once

#include "Common/DeviceResources.h"
#include "Common/StepTimer.h"

#include "Library/DebugFont.h"
#include "Library/SceneManager.h"
#include "UserResources.h"

#include "Common/TransitionMask.h"

//入力のマネージャー
#include "Library/InputManager.h"

#include <memory>


class Game final : public DX::IDeviceNotify
{
public:

    Game() noexcept(false);
    ~Game();

    Game(Game&&) = default;
    Game& operator= (Game&&) = default;

    Game(Game const&) = delete;
    Game& operator= (Game const&) = delete;

    // Initialization and management
    void Initialize(HWND window, int width, int height);

    // Basic game loop
    void Tick();

    // IDeviceNotify
    void OnDeviceLost() override;
    void OnDeviceRestored() override;

    // Messages
    void OnActivated();
    void OnDeactivated();
    void OnSuspending();
    void OnResuming();
    void OnWindowMoved();
    void OnDisplayChange();
    void OnWindowSizeChanged(int width, int height);

    // Properties
    void GetDefaultSize( int& width, int& height ) const noexcept;

private:

    void Update(DX::StepTimer const& timer);
    void Render();

    void Clear();

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

private:
    //フルスクリーン用
    BOOL m_fullscreen;

public:

    //画面モードを設定する関数
    void SetFullscreenState(BOOL value)
    {
        m_fullscreen = value;

        m_deviceResources->GetSwapChain()->SetFullscreenState(m_fullscreen, nullptr);

        if (value)
        {
            m_deviceResources->CreateWindowSizeDependentResources();
        }
    }

private:
    // Device resources.
    std::unique_ptr<DX::DeviceResources>    m_deviceResources;

    // Rendering loop timer.
    DX::StepTimer                           m_timer;

    
    //追加/////////////////////////////////////////////////////

    //共通ステートへのポインタ
    std::unique_ptr<DirectX::CommonStates> m_states;

    //デバッグ文字列の表示オブジェクト
    std::unique_ptr<DebugFont> m_debugFont;

    //キーボードのステートトラッカー
    DirectX::Keyboard::KeyboardStateTracker m_keybordTracker;

    //マウスのステートトラッカー
    DirectX::Mouse::ButtonStateTracker m_mouseTracker;

    //シーンのマネージャーのポインタ
    std::unique_ptr<SceneManager<UserResources>> m_sceneManager;

    //シーンへ渡すユーザー定義のリソースのポインタ
    std::unique_ptr<UserResources> m_userResources;

    //入力関連のマネージャー
    std::unique_ptr<InputManager> m_inputManager;


    //トランジションマスク
    std::unique_ptr<TransitionMask> m_transitionMask;
};
