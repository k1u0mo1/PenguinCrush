#include "pch.h"
#include "Library/StageManager.h"

//----------------------------------------------------------
// コンストラクタ
//----------------------------------------------------------

StageManager::StageManager(
    DX::DeviceResources* deviceResources)
    : m_deviceResources(deviceResources) 
{

}

//----------------------------------------------------------
// 初期化
//----------------------------------------------------------

void StageManager::Initialize(HWND /*hwnd*/, int /*width*/, int /*height*/)
{
    ////ステージクラスをインスタンスの生成
    //auto stage = std::make_unique<Stage>(m_deviceResources);

    ////ステージの初期化
    //stage->Initialize(hwnd, width, height);

    ////作成したステージを登録 ＜－大切
    //m_stages[L"DefaultStage"] = std::move(stage);

    ////登録したステージを現在のステージに設定
    //m_currentStage = m_stages[L"DefaultStage"].get();

    //初期化
    m_stages.clear();

    m_currentStage = nullptr;
}

//----------------------------------------------------------
// 新しいステージを生成しリストに追加して登録
//----------------------------------------------------------

void StageManager::AddStage(const std::wstring& name, HWND hwnd, int width, int height)
{
    auto stage = std::make_unique<Stage>(m_deviceResources);

    //ステージの初期化
    stage->Initialize(hwnd, width, height);

    //作成したステージを登録 ＜－大切
    m_stages[name] = std::move(stage);
}

//----------------------------------------------------------
// 指定した名前のステージに切り替える
//----------------------------------------------------------

void StageManager::ChangeStage(const std::wstring& name)
{
    //このステージはあるか？調べる
    auto it = m_stages.find(name);

    //ステージが見つかったときに差し替える
    if (it != m_stages.end())
    {
        m_currentStage = it->second.get();
    }
}

//----------------------------------------------------------
// 現在のステージを設定
//----------------------------------------------------------

void StageManager::SetCurrentStage(const std::wstring& name)
{
    ChangeStage(name);
}

//----------------------------------------------------------
// ステージの更新
//----------------------------------------------------------

void StageManager::Update(Wave* wave)
{
    if (m_currentStage)
    {
        m_currentStage->Update(wave);
    }
}

//----------------------------------------------------------
// ステージの描画
//----------------------------------------------------------

void StageManager::Render(ID3D11DeviceContext* context,
    const DirectX::SimpleMath::Matrix& view,
    const DirectX::SimpleMath::Matrix& proj,
    DisplayCollision* displayCollision)
{
    if (m_currentStage)
    {
        m_currentStage->Render(context, view, proj, displayCollision);
    }
}