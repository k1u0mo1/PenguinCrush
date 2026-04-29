//
// Fish.cpp
//

#include "pch.h"
#include "Fish.h"

#include <Effects.h>

using namespace DirectX;

using Microsoft::WRL::ComPtr;


using namespace DirectX;

//----------------------------------------------------------
// コンストラクタ
//----------------------------------------------------------

Fish::Fish(
    DX::DeviceResources* deviceResources,
    const DirectX::SimpleMath::Vector3& pos,
    const DirectX::SimpleMath::Vector3& dir,
    std::shared_ptr<DisplayCollision> displayCollision)
    : m_deviceResources(deviceResources)
    , m_position(pos)
    , m_direction(dir)
    , m_lifetime(0.0f)
    , m_displayCollision(displayCollision)
    , m_fishHeightOffset(0.0f)
{
    
}

//----------------------------------------------------------
// 魚の初期化
//----------------------------------------------------------

void Fish::Initialize()
{
    auto device = m_deviceResources->GetD3DDevice();

    // モデル読み込み
    EffectFactory fx(device);
    fx.SetDirectory(L"Resources\\Models");
    m_model =
        Model::CreateFromSDKMESH(
            device,
            L"Resources\\Models\\Fish.sdkmesh",
            fx
        );

    //真っ暗防止のため、強制的にライトを有効化＆白くする
    if (m_model)
    {
        for (auto& mesh : m_model->meshes)
        {
            for (auto& effect : mesh->meshParts)
            {
                auto lights = dynamic_cast<IEffectLights*>(effect.get());
                if (lights)
                {
                    lights->SetLightingEnabled(true);
                     //環境光を「白」に設定
                    lights->SetAmbientLightColor(Colors::White);
                }
            }
        }
    }
   
    m_sphere = DirectX::BoundingSphere(m_position, 1.0f);

    m_states = std::make_unique<CommonStates>(device);

}

//----------------------------------------------------------
// 魚の更新
//----------------------------------------------------------

void Fish::Update(
    float deltaTime, 
    const Stage* stage)
{
    //時間
	m_lifetime += deltaTime;

    //重力
    m_velocity.y += m_gravity * deltaTime;
    m_position.y += m_velocity.y * deltaTime;

    //ステージの高さを取得して沈み込みを再現
    float stageY = stage->GetGroundHeight(m_position.x, m_position.z);

    //ランダムな動き（乱数を生成）簡易的
    float randomVal = (float)(rand() % 100) / 100.0f;

    if (stage->IsInside(m_position.x, m_position.z))
    {
        //高さ調整
        m_fishHeightOffset = 0.0f;
        //魚がステージよりも下に落ちたときに補正する
        if (m_position.y <= stageY - m_fishHeightOffset)
        {
            m_position.y = stageY - m_fishHeightOffset;

            //落ちてきたとき
            if (m_velocity.y < 0)
            {
                //地面に当たったら跳ねるように
                m_velocity.y = -m_velocity.y * 0.5f;
            }

            //ペタペタ跳ねる演出
            //速度が下がると再度跳ねる
            if (abs(m_velocity.y) < 1.0f)
            {
                //〇％で小さく跳ねる
                if (rand() % 100 < 3)
                {
                    //3.0 ～ 5.0 の間
                    m_velocity.y = 3.0f + (randomVal * 2.0f);

                    //高さだけでなく位置もずらす
                    m_position.x += (randomVal - 5.0f) * 0.05f;
                }
                else
                {
                    //跳ねないとき
                    m_velocity.y = 0;
                }
            }
        }
    }

    // ステージの傾きを回転行列として適用
    //waveクラス→ステージ→魚へ傾きを取得（stage->GetRotateX()とstage->GetRotateZ()）
    SimpleMath::Matrix stageRotation =
        SimpleMath::Matrix::CreateRotationX(stage->GetRotateX()) *
        SimpleMath::Matrix::CreateRotationZ(stage->GetRotateZ());

    // Y軸上向きベクトルを回転
    //傾いたステージの法線ベクトルに変換
    SimpleMath::Vector3 normal =
        SimpleMath::Vector3::TransformNormal(
            SimpleMath::Vector3::Up, stageRotation);
    normal.Normalize();

    //重力ベクトルを定義　常に（0,-1,0）
    SimpleMath::Vector3 gravity = SimpleMath::Vector3::Down;

    


    //接している平面方向（重力を法線に投影して引く）
    //gravity.Dot(normal)で重力のうち法線方向の成分を計算する
    //gravityで引いて接平面上の成分を取る
    //slideDir = 滑る方向
    SimpleMath::Vector3 slideDir = gravity - (gravity.Dot(normal) * normal);
    
    m_sligeBehavior.Update(
        m_position,
        m_velocity,
        slideDir,
        deltaTime
    );


    ////十分に傾いている時に滑る
    //if (slideDir.LengthSquared() > 0.0001f)
    //{
    //    slideDir.Normalize();
    //    //強さを設定
    //    //acos(normal.y)で水平から傾斜角度を出す
    //    float slopeAngle = acos(std::max(-1.0f, std::min(1.0f, normal.y)));
    //    //sinf(slopeAngle)はどれだけ横に滑るか？
    //    float slideStrength = sinf(slopeAngle) * 50.0f;//数値は大きめに
    //    //魚の位置を更新する
    //    //傾斜角に応じた速度で魚を滑らせる
    //    m_position += slideDir * slideStrength * deltaTime;
    //}
    ////OBB更新
    //if (m_collision)
    //{
    //    //m_collision->SetCenter(m_position);
    //}

    m_sphere.Center = m_position;
}

//----------------------------------------------------------
// 魚の描画
//----------------------------------------------------------

void Fish::Render(
    ID3D11DeviceContext* context, 
    const DirectX::SimpleMath::Matrix& view, 
    const DirectX::SimpleMath::Matrix& proj,
    ShadowRenderer* shadowRenderer,
    const Stage* stage)
{
   
    //---------------------------------------------------
    //影の描画
    //---------------------------------------------------

    if (shadowRenderer && stage)
    {
        //影の座標
        DirectX::SimpleMath::Vector3 shadowPos = m_position;

        //地面の高さ
        float groundY = stage->GetGroundHeight(m_position.x, m_position.z);

        //取得した地面の高さをセット
        shadowPos.y = groundY;

        //影の大きさ
        float shadowScale = 1.5f;

        //ステージの傾き
        float rotX = stage->GetRotateX();
        float rotZ = stage->GetRotateZ();

        //影の描画
        shadowRenderer->Render(
            context,
            m_states.get(),
            view,
            proj,
            shadowPos,
            shadowScale,
            rotX,    
            rotZ     
        );
    }


    if (!m_model||!m_states) return; // 安全チェック

    //正規化
    SimpleMath::Vector3 forward = m_direction;
    forward.Normalize();
    
    SimpleMath::Vector3 up = SimpleMath::Vector3(0, 1, 0);
    SimpleMath::Vector3 right = up.Cross(forward);
    right.Normalize();
    SimpleMath::Vector3 actualUp = forward.Cross(right);

    SimpleMath::Matrix orientation = SimpleMath::Matrix(
        right.x, right.y, right.z, 0,
        actualUp.x, actualUp.y, actualUp.z, 0,
        forward.x, forward.y, forward.z, 0,
        0, 0, 0, 1
    );

    // スケールと移動行列 0.4f, 0.3f, 0.3f
    DirectX::SimpleMath::Matrix scale = 
        DirectX::SimpleMath::Matrix::CreateScale(FISH_SCALE_X, FISH_SCALE_Y, FISH_SCALE_Z);

    //回転
    SimpleMath::Matrix initRotX = SimpleMath::Matrix::CreateRotationZ(XMConvertToRadians(270.0f));
    SimpleMath::Matrix initRotY = SimpleMath::Matrix::CreateRotationY(XMConvertToRadians(MODEL_ROTATION_OFFSET));

    //ピチピチ動くアニメーション
    //sin波で時間経過でZ軸を振動させる
    //m_lifetime＊FLOP_SPEEDで高速に振動
    float flopAngle = sinf(m_lifetime * FLOP_SPEED) * XMConvertToRadians(15.0f);

    //揺れ 
    SimpleMath::Matrix flopRot = SimpleMath::Matrix::CreateRotationX(flopAngle);

    //揺れ 左右
    SimpleMath::Matrix flopRotZ = SimpleMath::Matrix::CreateRotationZ(flopAngle);

    //座標
    DirectX::SimpleMath::Matrix trans =
        DirectX::SimpleMath::Matrix::CreateTranslation(m_position);
    
    // 最終ワールド行列
    DirectX::SimpleMath::Matrix world =
        scale * initRotX * initRotY * flopRot * flopRotZ * orientation * trans;

    context->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
    context->OMSetDepthStencilState(m_states->DepthDefault(), 0);
    context->RSSetState(m_states->CullNone());

    //モデルの描画
    m_model->Draw(context, *m_states, world, view, proj);

    //// デバッグ描画
    //if (m_collision && m_displayCollision)
    //{
    //    BoundingBox box;
    //    box.Center = m_collision->GetCenter();
    //    box.Extents = m_collision->GetExtents();
    //    
    //    //コリジョン線
    //    //m_displayCollision->AddBoundingBox(box, Colors::Red);
    //    
    //}


    //当たり判定の描画-----------------------------------------
    if (m_displayCollision)
    {
        m_displayCollision->AddBoundingSphere(m_sphere, Colors::Red);
    }
}

//----------------------------------------------------------
// 魚が現在生存しているか確認
//----------------------------------------------------------

bool Fish::IsAlive() const
{
	return m_lifetime < MAX_LIFETIME;
}

//----------------------------------------------------------
// 弾が当たった際などに呼ばれ、魚を消滅
//----------------------------------------------------------

void Fish::BulletKill()
{
    m_lifetime = MAX_LIFETIME;
}

//----------------------------------------------------------
// 魚の現在位置を取得
//----------------------------------------------------------

DirectX::SimpleMath::Vector3 Fish::GetPosition() const
{
    return m_position;
}

//----------------------------------------------------------
// 魚の当たり判定コリジョン　箱
//----------------------------------------------------------

DirectX::BoundingBox Fish::GetBoundingBox() const
{
    BoundingBox box;
    if (m_collision)
    {
        box.Center = m_collision->GetCenter();
        box.Extents = m_collision->GetExtents();
    }
    else
    {
        // デフォルト小さい箱
        box.Center = m_position;
        box.Extents = SimpleMath::Vector3(1.0f, 1.0f, 2.0f);
    }
    return box;
}



