//
// BulletE.cpp
//

#include "pch.h"
#include "BulletE.h"

#include <Effects.h>

#include "ShadowRenderer/ShadowRenderer.h"

using namespace DirectX;

using Microsoft::WRL::ComPtr;

BulletE::BulletE(
    DX::DeviceResources* deviceResources,
    const DirectX::SimpleMath::Vector3& pos,
    const DirectX::SimpleMath::Vector3& dir,
    std::shared_ptr<DisplayCollision> displayCollision,
    std::shared_ptr<DirectX::Model> model,
    float speed )

    : m_deviceResources(deviceResources)
    , m_position(pos)
    , m_direction(dir)
    , m_speed(speed)
    , m_lifetime(0.0f)
    , m_displayCollision(displayCollision)
    , m_isAlive(true)
    , m_bulletModel(model)
    , m_bulletHeightOffset(0.0f)
{

    // CommonStatesの作成
    m_states = std::make_unique<CommonStates>(m_deviceResources->GetD3DDevice());

    // 当たり判定の作成
    if (m_bulletModel)
    {
        m_collision = std::make_unique<ModelCollisionOrientedBox>(m_bulletModel.get());
        m_collision->SetCenter(m_position);
        // サイズ調整（必要に応じて）
        m_collision->SetExtents(SimpleMath::Vector3(2.0f, 2.0f, 3.5f));
    }
}



void BulletE::Update(float deltaTime)
{
    // 通常の移動処理のみを行う 
    m_position += m_direction * m_speed * SPEED_MULTIPLIER * deltaTime;
   
    m_lifetime += deltaTime;

    if (m_collision)
    {
        m_collision->SetCenter(m_position);
    }
}

void BulletE::Update(
    float deltaTime, 
    const Stage* stage)
{
   
    //直線移動
    DirectX::SimpleMath::Vector3 oldPos = m_position;
	
    m_position += m_direction * m_speed * SPEED_MULTIPLIER * deltaTime;
	
    m_lifetime += deltaTime;

    //移動後の位置の地形の高さ
    float stageY_old = stage->GetGroundHeight(oldPos.x, oldPos.z);
    float stageY_new = stage->GetGroundHeight(m_position.x, m_position.z);


    //地形にぶつかったか？
    if ((oldPos.y >= stageY_old && m_position.y <= stageY_new) ||
        (oldPos.y <= stageY_old && m_position.y >= stageY_new))
    {
        //ステージとぶつかった
        m_isAlive = false;
        return;
    }

    //OBB更新
    if (m_collision)
    {
        m_collision->SetCenter(m_position);
    }
}



void BulletE::Render(
    ID3D11DeviceContext* context, 
    const DirectX::SimpleMath::Matrix& view, 
    const DirectX::SimpleMath::Matrix& proj
    )
{

    if (!m_bulletModel) return; // 安全チェック

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

    // スケールと移動行列
    DirectX::SimpleMath::Matrix scale = 
        DirectX::SimpleMath::Matrix::CreateScale(0.4f, 0.3f, 0.3f);
    
    //回転
    SimpleMath::Matrix initRotY = SimpleMath::Matrix::CreateRotationY(XMConvertToRadians(MODEL_ROTATION_Y));

    DirectX::SimpleMath::Matrix trans =
        DirectX::SimpleMath::Matrix::CreateTranslation(m_position);
    
    // 最終ワールド行列
    DirectX::SimpleMath::Matrix world = scale* initRotY * orientation * trans;

    m_bulletModel->Draw(context, *m_states.get(), world, view, proj);

    // デバッグ描画
    if (m_collision && m_displayCollision)
    {
        BoundingBox box;
        box.Center = m_collision->GetCenter();
        box.Extents = m_collision->GetExtents();

        //コリジョン線
        m_displayCollision->AddBoundingBox(box, Colors::Red);
    }
}

bool BulletE::IsAlive() const
{
	return m_isAlive&&( m_lifetime < MAX_LIFETIME);
}

//void BulletE::BulletKill()
//{
//    m_lifetime = MAX_LIFETIME;
//}



//DirectX::SimpleMath::Vector3 BulletE::GetPosition() const
//{
//    return m_position;
//}

DirectX::BoundingBox BulletE::GetBoundingBox() const
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



