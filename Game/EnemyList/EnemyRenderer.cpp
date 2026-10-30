
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

	//状態によって描画するモデルを切り替える
    DirectX::Model* currentModel = nullptr;
    switch (state)
    {
		//待機状態と同じモデルを使用
    case EnemyState::Opening:
    case EnemyState::Loading:
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
        
        //気絶
    case EnemyState::Dizzy:
        currentModel = m_modelIdle;
        break;

    default:
        currentModel = m_modelIdle;
        break;
	}

	//モデルがない場合は描画しない
    if (!currentModel) return;

    //気絶状態の時、敵をふらふら揺らす計算
    DirectX::SimpleMath::Matrix finalRotaion = rotationMatrix;
    if (state == EnemyState::Dizzy)
    {
        //時間経過で左右に揺らす
        float swayZ = sinf(dizzyRotationY * DIZZY_SWAY_SPEED) * DIZZY_SWAY_ANGLE;
        float swayX = cosf(dizzyRotationY * DIZZY_SWAY_SPEED * 0.5f) * (DIZZY_SWAY_ANGLE * 0.5f);

        DirectX::SimpleMath::Matrix swatMatrix =
            DirectX::SimpleMath::Matrix::CreateRotationX(swayX) *
            DirectX::SimpleMath::Matrix::CreateRotationZ(swayZ);

        finalRotaion = swatMatrix * rotationMatrix;
    }
    //死亡演出　タイマーの進捗に合わせて後ろに倒れる
    else if (state == EnemyState::Dying)
    {
        //引数等を経由してタイマー値を受け取る
        float progress = std::min(dizzyRotationY / 1.0f, 1.0f);
        //倒れながら回転する
        float spinAngle = progress * DirectX::XM_2PI * 2.0f;
        //後ろに倒れる
        float pitchAngle = DirectX::XMConvertToRadians(90.0f * progress);

        //回転行列
        DirectX::SimpleMath::Matrix spinMatrix = 
            DirectX::SimpleMath::Matrix::CreateRotationY(spinAngle);

        DirectX::SimpleMath::Matrix pitchMatrix =
            DirectX::SimpleMath::Matrix::CreateRotationX(pitchAngle);

        //合成
        finalRotaion = (spinMatrix * pitchMatrix) * rotationMatrix;
    }

	//回転行列と平行移動行列を組み合わせてワールド行列を作成
    DirectX::SimpleMath::Matrix world =
        finalRotaion * DirectX::SimpleMath::Matrix::CreateTranslation(position);

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

    //気絶状態の時に、敵の頭上にエフェクトモデルを置く
    if (state == EnemyState::Dizzy && m_materialDizzy)
    {
        //頭上に配置する座標
        DirectX::SimpleMath::Vector3 dizzyPos = position;
        dizzyPos.y += DIZZY_EFFECT_OFFSET_Y;

        //頭上で回転する
        DirectX::SimpleMath::Matrix dizzyWorld =
            DirectX::SimpleMath::Matrix::CreateRotationY(dizzyRotationY) *
            DirectX::SimpleMath::Matrix::CreateTranslation(dizzyPos);

        m_materialDizzy->Draw(context, *m_states, dizzyWorld, view, proj);
    }

    
}
