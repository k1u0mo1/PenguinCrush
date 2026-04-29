#include "pch.h"
#include "PlayerRenderer.h"

#include "DDSTextureLoader.h"

using namespace DirectX;

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
    m_states = std::make_unique<CommonStates>(device);

    DirectX::EffectFactory fx(device);
    fx.SetDirectory(L"Resources\\Models");

    //各状態のモデルをロード
    m_modelIdle     = Model::CreateFromSDKMESH(device, L"Resources\\Models\\Pen_Stand.sdkmesh", fx);
    m_modelAttack   = Model::CreateFromSDKMESH(device, L"Resources\\Models\\PenAttack.sdkmesh", fx);
    m_modelShoot    = Model::CreateFromSDKMESH(device, L"Resources\\Models\\Pen_Shoot.sdkmesh", fx);
    m_modelRush     = Model::CreateFromSDKMESH(device, L"Resources\\Models\\Pen_Rush.sdkmesh",  fx);
    m_materialDizzy = Model::CreateFromSDKMESH(device, L"Resources\\Models\\Fainting.sdkmesh",  fx);

    //影用テクスチャの読み込み
    CreateDDSTextureFromFile(device, L"Resources\\Textures\\Shadow.dds", nullptr, m_shadowTexture.GetAddressOf());
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
    Model* currentModel = m_modelIdle.get();

	//ふらつき状態は通常モデルを使用
    switch (state)
    {
    case Player::PlayerState::Attack: currentModel = m_modelAttack.get(); break;
    case Player::PlayerState::Shoot:  currentModel = m_modelShoot.get();  break;
    case Player::PlayerState::Rush:   currentModel = m_modelRush.get();   break;
    case Player::PlayerState::Dizzy:  currentModel = m_modelIdle.get();   break;
    default:                          currentModel = m_modelIdle.get();   break;
    }

    if (!currentModel) return;

	//前方向ベクトルからY軸の回転角を計算
    float angleY = atan2(forward.x, forward.z);
	//モデルの向きを調整するために180度回転
    SimpleMath::Matrix rot = SimpleMath::Matrix::CreateRotationY(angleY + DirectX::XM_PI);

	//ふらつき状態の揺れの計算
    SimpleMath::Matrix dizzySway = SimpleMath::Matrix::Identity;
	//ふらつき状態のときは、サイン波を使ってZ軸回りに揺れる行列を作成
    if (state == Player::PlayerState::Dizzy)
    {
		//サイン波を使って揺れの角度を計算
        float swayAngle = sinf(dizzyRotationY * DIZZY_SWAY_SPEED) * DIZZY_SWAY_ANGLE;
		
        //Z軸回りの回転行列を作成
        dizzySway = SimpleMath::Matrix::CreateRotationZ(swayAngle);
    }

	//ワールド行列の計算
    SimpleMath::Matrix world = 
        rotationMatrix * dizzySway * rot * SimpleMath::Matrix::CreateTranslation(position);

    //影の描画
    if (stage && shadowRenderer && m_shadowTexture)
    {
		//影の位置はプレイヤーの位置と同じX,ZでYを少し下げる
        SimpleMath::Vector3 shadowPos = position;

		//影の大きさ
        float shadowScale = 2.0f;

		//ステージの傾きに合わせて影も回転させる
        shadowRenderer->Render(
            context, 
            m_states.get(),
            view, 
            proj, 
            shadowPos,
            shadowScale,
            stage->GetRotateX(),
            stage->GetRotateZ());
    }

    //プレイヤーの描画
    currentModel->Draw(context, *m_states, world, view, proj);

    //ふらつきエフェクトの描画
    if (state == Player::PlayerState::Dizzy && m_materialDizzy)
    {
		//エフェクトの位置はプレイヤーの位置と同じX,ZでYを少し上げる
        SimpleMath::Matrix birdTrans = SimpleMath::Matrix::CreateTranslation(position.x, position.y + DIZZY_EFFECT_OFFSET_Y, position.z);
		//エフェクトの大きさは0.5倍
        SimpleMath::Matrix birdScale = SimpleMath::Matrix::CreateScale(0.5f);
		//エフェクトの回転はY軸でふらつきの回転に加えてさらに回転させる
        SimpleMath::Matrix birdRot = SimpleMath::Matrix::CreateRotationY(dizzyRotationY);
		//ワールド行列の計算
        SimpleMath::Matrix birdWorld = birdScale * birdRot * birdTrans;
		//エフェクトの描画
        m_materialDizzy->Draw(context, *m_states, birdWorld, view, proj);
    }
}
