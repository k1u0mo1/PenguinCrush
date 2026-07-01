
/**
 * @file   PlayerRenderer.cpp
 * @brief  プレイヤーキャラクターの描画を行うクラス
 * @author 國田知睦
 * @date   2026/07/01
 */

#include "pch.h"
#include "PlayerRenderer.h"
#include "Game/Common/ObjectCharacter/ModelManager.h"
#include "DDSTextureLoader.h"

//-----------------------------------------------------------------
// コンストラクタ
//-----------------------------------------------------------------

PlayerRenderer::PlayerRenderer()
{
}

//-----------------------------------------------------------------
// モデルやテクスチャなどのリソースを初期化する
//-----------------------------------------------------------------

void PlayerRenderer::Initialize(ID3D11Device* device)
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

void PlayerRenderer::Render(
    ID3D11DeviceContext* context,
    const DirectX::SimpleMath::Vector3& position,
    const DirectX::SimpleMath::Vector3& forward,
    Player::PlayerState state,
    float dizzyRotationY, 
    const DirectX::SimpleMath::Matrix& rotationMatrix,
    const DirectX::SimpleMath::Matrix& view,
    const DirectX::SimpleMath::Matrix& proj,
    Stage* stage, 
    ShadowRenderer* shadowRenderer)
{
	//状態に応じたモデルを選択
    DirectX::Model* currentModel = m_modelIdle;

	//ふらつき状態は通常モデルを使用
    switch (state)
    {
    case Player::PlayerState::Attack: currentModel = m_modelAttack; break;
    case Player::PlayerState::Rush:   currentModel = m_modelRush;   break;
    case Player::PlayerState::Dizzy:  currentModel = m_modelIdle;   break;
    default:                          currentModel = m_modelIdle;   break;
    }

    if (!currentModel) return;

	//前方向ベクトルからY軸の回転角を計算
    float angleY = atan2(forward.x, forward.z);
	//モデルの向きを調整するために180度回転
    DirectX::SimpleMath::Matrix rot = DirectX::SimpleMath::Matrix::CreateRotationY(angleY + DirectX::XM_PI);

	//ふらつき状態の揺れの計算
    DirectX::SimpleMath::Matrix dizzySway = DirectX::SimpleMath::Matrix::Identity;
	//ふらつき状態のときは、サイン波を使ってZ軸回りに揺れる行列を作成
    if (state == Player::PlayerState::Dizzy)
    {
		//サイン波を使って揺れの角度を計算
        float swayAngle = sinf(dizzyRotationY * DIZZY_SWAY_SPEED) * DIZZY_SWAY_ANGLE;
		
        //Z軸回りの回転行列を作成
        dizzySway = DirectX::SimpleMath::Matrix::CreateRotationZ(swayAngle);
    }

	//ワールド行列の計算
    DirectX::SimpleMath::Matrix world =
        rotationMatrix * dizzySway * rot * DirectX::SimpleMath::Matrix::CreateTranslation(position);

    //影の描画
    if (stage && shadowRenderer && m_shadowTexture)
    {
		//影の位置はプレイヤーの位置と同じX,ZでYを少し下げる
        DirectX::SimpleMath::Vector3 shadowPos = position;

		//ステージの傾きに合わせて影も回転させる
        shadowRenderer->Render(
            context, 
            m_states.get(),
            view, 
            proj, 
            shadowPos,
            SHADOW_SCALE,
            stage->GetRotateX(),
            stage->GetRotateZ());
    }

    //プレイヤーの描画
    currentModel->Draw(context, *m_states, world, view, proj);

    //ふらつきエフェクトの描画
    if (state == Player::PlayerState::Dizzy && m_materialDizzy)
    {
		//エフェクトの位置はプレイヤーの位置と同じX,ZでYを少し上げる
        DirectX::SimpleMath::Matrix birdTrans = 
            DirectX::SimpleMath::Matrix::CreateTranslation(position.x, position.y + DIZZY_EFFECT_OFFSET_Y, position.z);
        //エフェクトの大きさ
        DirectX::SimpleMath::Matrix birdScale = 
            DirectX::SimpleMath::Matrix::CreateScale(DIZZY_EFFECT_SCALE);
        //エフェクトの回転はY軸でふらつきの回転に加えてさらに回転させる
        DirectX::SimpleMath::Matrix birdRot =
            DirectX::SimpleMath::Matrix::CreateRotationY(dizzyRotationY);
        //ワールド行列の計算
        DirectX::SimpleMath::Matrix birdWorld = birdScale * birdRot * birdTrans;
        //エフェクトの描画
        m_materialDizzy->Draw(context, *m_states, birdWorld, view, proj);
    }
}
