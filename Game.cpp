//
// Game.cpp
//

#include "pch.h"
#include "Game.h"

//シーンクラスの一覧///////////////////////////////////////

#include "Game/SceneList/TitleScene.h"//タイトル
#include "Game/SceneList/SelectScene.h"//ゲームの選択
#include "Game/SceneList/GamePlayScene.h"//ゲームプレイ
#include "Game/SceneList/ResultScene.h"//リザルト

#include "Game/SoundList/AudioManager.h"

///////////////////////////////////////////////////////////

extern void ExitGame() noexcept;

using namespace DirectX;

using Microsoft::WRL::ComPtr;

Game::Game() noexcept(false)
    
{
    m_deviceResources = std::make_unique<DX::DeviceResources>();
    // TODO: Provide parameters for swapchain format, depth/stencil format, and backbuffer count.
    //   Add DX::DeviceResources::c_AllowTearing to opt-in to variable rate displays.
    //   Add DX::DeviceResources::c_EnableHDR for HDR10 display.
    m_deviceResources->RegisterDeviceNotify(this);

    m_fullscreen = FALSE;



}

Game::~Game()
{
    AudioManager::GetInstance()->Suspend();

    //シーンマネージャーを明示的に解放する
    //これにより GamePlayScene 内の SoundEffectInstance (再生中の音) が
    //エンジン本体より先に片付けられます
    m_sceneManager.reset();
}

// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND window, int width, int height)
{
    //-------------------------------------------------
    //効果音の初期化
    //-------------------------------------------------

    // オーディオ初期化
    AudioManager::GetInstance()->Initialize();



    m_deviceResources->SetWindow(window, width, height);

    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

    // TODO: Change the timer settings if you want something other than the default variable timestep mode.
    // e.g. for 60 FPS fixed timestep update logic, call:
    /*
    m_timer.SetFixedTimeStep(true);
    m_timer.SetTargetElapsedSeconds(1.0 / 60);
    */

    /*auto device = m_deviceResources->GetD3DDevice();
    auto context = m_deviceResources->GetD3DDeviceContext();*/

    //////////////////////////////////////////////////////////

    //マウスカーソルを消す
    ShowCursor(FALSE);


    //どのシーンにするか
    m_sceneManager->SetScene<TitleScene>();
    //m_sceneManager->SetScene<GamePlayScene>();
    
    //
    //context->ClearRenderTargetView(m_transtionTexture->GetRenderTargetView(), Colors::Black);

    //入力関連のマネージャーの初期化
    m_inputManager = std::make_unique<InputManager>();
    
    m_userResources->SetInputManager(m_inputManager.get());
    
    
    
    ////////////////////////////////////////////////////////
}

#pragma region Frame Update
// Executes the basic game loop.
void Game::Tick()
{
    m_timer.Tick([&]()
    {
        Update(m_timer);
    });

    Render();
}

// Updates the world.
void Game::Update(DX::StepTimer const& timer)
{
    float elapsedTime = float(timer.GetElapsedSeconds());

    // TODO: Add your game logic here.
    elapsedTime;

    ///////////////////////////////////////////////////

    //入力更新(毎フレーム)
    m_inputManager->Update();

    auto input = m_inputManager.get();

    // Escapeキーでゲーム終了
    if (input->kbTracker.pressed.Escape)
    {
        PostQuitMessage(0);
        return; 
    }


    //-------------------------------------------------
    //効果音の更新
    //-------------------------------------------------

    AudioManager::GetInstance()->Update();

    //シーンのupdate
    m_sceneManager->Update(elapsedTime);

    if (m_transitionMask)
    {
        m_transitionMask->Update(static_cast<float>(timer.GetElapsedSeconds()));
    }

    ///////////////////////////////////////////////////
}
#pragma endregion

#pragma region Frame Render
// Draws the scene.
void Game::Render()
{
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    Clear();

    m_deviceResources->PIXBeginEvent(L"Render");
    auto context = m_deviceResources->GetD3DDeviceContext();

    // TODO: Add your rendering code here.
    context;

    ///////////////////////////////////////////////////////////////////
    
    //シーンのRender
    m_sceneManager->Render();

    //フェードの描画
    if (m_transitionMask)
    {
        // 画面サイズを取得して描画
        RECT rect = m_deviceResources->GetOutputSize();
        m_transitionMask->Draw(
            m_deviceResources->GetD3DDeviceContext(),
            m_states.get(),
            nullptr,
            rect
        );
    }

    ////ゲームのFPS表示-------------------------------
    //std::wostringstream oss;
    //oss << "fps:" << m_timer.GetFramesPerSecond();
    //m_debugFont->AddString(oss.str().c_str(),
    //    SimpleMath::Vector2(0.0f, 0.0f));

    //m_debugFont->Render(m_states.get());
    ////----------------------------------------------



    ///////////////////////////////////////////////////////////////////

    m_deviceResources->PIXEndEvent();

    // Show the new frame.
    m_deviceResources->Present();
}

// Helper method to clear the back buffers.
void Game::Clear()
{
    m_deviceResources->PIXBeginEvent(L"Clear");

    // Clear the views.
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto renderTarget = m_deviceResources->GetRenderTargetView();
    auto depthStencil = m_deviceResources->GetDepthStencilView();

    context->ClearRenderTargetView(renderTarget, Colors::CornflowerBlue);
    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->OMSetRenderTargets(1, &renderTarget, depthStencil);

    // Set the viewport.
    const auto viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    m_deviceResources->PIXEndEvent();
}
#pragma endregion

#pragma region Message Handlers
// Message handlers
void Game::OnActivated()
{
    // TODO: Game is becoming active window.
}

void Game::OnDeactivated()
{
    // TODO: Game is becoming background window.
}

void Game::OnSuspending()
{
    // TODO: Game is being power-suspended (or minimized).
}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();

    // TODO: Game is being power-resumed (or returning from minimize).
}

void Game::OnWindowMoved()
{
    // デバイスリソース、またはスワップチェーンがまだない場合は何もしないで帰る
    if (!m_deviceResources || !m_deviceResources->GetSwapChain())
    {
        return;
    }

    const auto r = m_deviceResources->GetOutputSize();
    m_deviceResources->WindowSizeChanged(r.right, r.bottom);

    //フルスクリーンか調べる
    BOOL fullscreen = FALSE;

    m_deviceResources->GetSwapChain()->GetFullscreenState(&fullscreen, nullptr);

    //フルスクリーンが解除されてしまったときの処理
    if (m_fullscreen != fullscreen)
    {
        m_fullscreen = fullscreen;

        //ResizeBuffers関数を呼び出す
        m_deviceResources->CreateWindowSizeDependentResources();
    }

}

void Game::OnDisplayChange()
{
    m_deviceResources->UpdateColorSpace();
}

void Game::OnWindowSizeChanged(int width, int height)
{
    if (!m_deviceResources->WindowSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();

    // TODO: Game window is being resized.
}

// Properties
void Game::GetDefaultSize(int& width, int& height) const noexcept
{
    // TODO: Change to desired default window size (note minimum size is 320x200).
    width = 1280;
    height = 720;
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Game::CreateDeviceDependentResources()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto context = m_deviceResources->GetD3DDeviceContext();

    // TODO: Initialize device dependent objects here (independent of window size).
    device;

    //////////////////////////////////////////////////////

    //ステートを作成
    m_states = std::make_unique<CommonStates>(device);

    //フォントの読み込み
    m_debugFont = std::make_unique<DebugFont>
        (device, context, L"Resources/Font/SegoeUI_18.spritefont");

    //ユーザーリソースの作成
    if (!m_userResources) 
        m_userResources = std::make_unique<UserResources>();

    //シーンマネージャーの作成
    if (!m_sceneManager)
        m_sceneManager = std::make_unique < SceneManager<UserResources>>(m_userResources.get());

    //TransitionMask を作成して登録する
    m_transitionMask = std::make_unique<TransitionMask>(device, context, 1.0f);
    m_userResources->SetTransitionMask(m_transitionMask.get());

    //ユーザーリソース関連-------------------------------------------
    m_userResources->SetDeviceResources(m_deviceResources.get());
    m_userResources->SetCommonStates(m_states.get());
    m_userResources->SetDebugFont(m_debugFont.get());
    m_userResources->SetKeyboardStateTracker(&m_keybordTracker);
    m_userResources->SetMouseStateTracker(&m_mouseTracker);
    m_userResources->SetStepTimerStates(&m_timer);
    //---------------------------------------------------------------

    //各シーンのリソースを持ってくる
    m_sceneManager->CreateDeviceDependentResources();

    ///////////////////////////////////////////////////////
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
    // TODO: Initialize windows-size dependent objects here.
    m_sceneManager->CreateWindowSizeDependentResources();
}

void Game::OnDeviceLost()
{
    // TODO: Add Direct3D resource cleanup here.
    m_sceneManager->OnDeviceLost();
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
#pragma endregion
