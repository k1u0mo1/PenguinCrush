
/**
 * @file   EnemyRenderer.cpp
 * @brief  敵の描画関連の管理を行うクラス
 * @author 國田知睦
 * @date   2026/07/16
 */

#include "pch.h"
#include "EnemyRenderer.h"
#include "Game/Common/ObjectCharacter/ModelManager.h"
#include "DDSTextureLoader.h"

//-----------------------------------------------------------------
// コンストラクタ
//-----------------------------------------------------------------

EnemyRenderer::EnemyRenderer()
{
}

//-----------------------------------------------------------------
// モデルやテクスチャなどのリソースを初期化する
//-----------------------------------------------------------------

void EnemyRenderer::Initialize(ID3D11Device* device)
{
	//共通ステートの初期化
    m_states = std::make_unique<DirectX::CommonStates>(device);

    DirectX::EffectFactory fx(device);
    fx.SetDirectory(L"Resources\\Models");

	//モデルマネージャーからモデルを取得
    m_modelIdle = ModelManager::GetInstance()->GetIdleModel();
    m_modelAttack = ModelManager::GetInstance()->GetAttackModel();
    m_modelRush = ModelManager::GetInstance()->GetRushModel();
    m_materialDizzy = ModelManager::GetInstance()->GetDizzyMaterial();

    //影用テクスチャの読み込み
    DirectX::CreateDDSTextureFromFile(device, L"Resources\\Textures\\Shadow.dds", nullptr, m_shadowTexture.GetAddressOf());
}

//-----------------------------------------------------------------
// モデルを描画する
//-----------------------------------------------------------------

void EnemyRenderer::Render(
    ID3D11DeviceContext* context,
    const DirectX::SimpleMath::Vector3& position,
    const DirectX::SimpleMath::Vector3& forward,
    EnemyState state,
    float dizzyRotationY, 
    const DirectX::SimpleMath::Matrix& rotationMatrix,
    const DirectX::SimpleMath::Matrix& view,
    const DirectX::SimpleMath::Matrix& proj,
    Stage* stage, 
    ShadowRenderer* shadowRenderer)
{
    UNREFERENCED_PARAMETER(forward);
    UNREFERENCED_PARAMETER(dizzyRotationY);

	//状態によって描画するモデルを切り替える
    DirectX::Model* currentModel = nullptr;
    switch (state)
    {
		//待機状態と同じモデルを使用
    case EnemyState::Opening:
    case EnemyState::Loading:
        currentModel = m_modelIdle;
		break;
        //通常
    case EnemyState::Idle:
        currentModel = m_modelIdle;
        break;
        //攻撃
    case EnemyState::Attack:
        currentModel = m_modelAttack;
        break;
        //突進
    case EnemyState::Rush:
        currentModel = m_modelRush;
        break;
    
    default:
        currentModel = m_modelIdle;
        break;
	}

	//モデルがない場合は描画しない
    if (!currentModel) return;

	//回転行列と平行移動行列を組み合わせてワールド行列を作成
    DirectX::SimpleMath::Matrix world =
        rotationMatrix * DirectX::SimpleMath::Matrix::CreateTranslation(position);

    //影の描画
    if (stage && shadowRenderer)
    {
		shadowRenderer->Render(
            context, 
            m_states.get(),
            view, proj, position,
            SHADOW_DEFAULT_SCALE, SHADOW_DEFAULT_ROT_X, SHADOW_DEFAULT_ROT_Z);
    }
    
    //モデルの描画
    currentModel->Draw(context, *m_states, world, view, proj);
}
