#include "pch.h"
#include "Bullet.h"

#include <Effects.h>
using namespace DirectX::SimpleMath;
using namespace DirectX;

Bullet::Bullet(
    DX::DeviceResources* deviceResources,
    const Vector3& pos,
    const Vector3& dir,
    const Stage* stage,
    std::shared_ptr<DisplayCollision> displayCollision,
    float speed,
    float damage
)
    : m_deviceResources(deviceResources)
    , m_position(pos)
    , m_direction(dir)
    , m_stage(stage)
    , m_displayCollision(displayCollision)
    , m_speed(speed)
    , m_damage(damage)
    , m_lifetime(0.0f)
    , m_isAlive(true)
{
    auto device = m_deviceResources->GetD3DDevice();

    EffectFactory fx(device);
    fx.SetDirectory(L"Resources\\Models");

    // モデル読み込み
    m_bulletModel =
        Model::CreateFromSDKMESH(
            device,
            L"Resources\\Models\\Cube.sdkmesh",
            fx
        );
    //-----------------------------
    // OBB
    //-----------------------------
    if (m_bulletModel)
    {
        m_collision = std::make_unique<ModelCollisionOrientedBox>(m_bulletModel.get());
        m_collision->SetCenter(m_position);
        m_collision->SetExtents(Vector3(0.25f, 0.25f, 0.25f));
    }
}

void Bullet::Update(float dt)
{
    if (!m_isAlive) return;

    Vector3 oldPos = m_position;
    m_position += m_direction * m_speed * dt;
    m_lifetime += dt;

    //-----------------------------------------------------------
    // 弾の寿命チェック
    //-----------------------------------------------------------
    if (m_lifetime >= MAX_LIFETIME)
    {
        m_isAlive = false;
        return;
    }

    //-----------------------------------------------------------
    // ステージ衝突
    //-----------------------------------------------------------
    if (m_stage)
    {
        float yOld = m_stage->GetGroundHeight(oldPos.x, oldPos.z);
        float yNew = m_stage->GetGroundHeight(m_position.x, m_position.z);

        if ((oldPos.y > yOld && m_position.y <= yNew) ||
            (oldPos.y < yOld && m_position.y >= yNew))
        {
            m_isAlive = false;
            return;
        }
    }
    //-----------------------------------------------------------
    // OBB更新
    //-----------------------------------------------------------

    if (m_collision)
    {
        m_collision->SetCenter(m_position);
    }
}

void Bullet::Render(
    ID3D11DeviceContext* context,
    const Matrix& view,
    const Matrix& proj)
{
    auto device = m_deviceResources->GetD3DDevice();

    DirectX::CommonStates states(device);

    if (!m_bulletModel || !m_isAlive) return;

    // 向きの行列
    Vector3 f = m_direction; f.Normalize();
    Vector3 up(0, 1, 0);
    Vector3 r = up.Cross(f); r.Normalize();
    Vector3 u = f.Cross(r);

    Matrix orientation(
        r.x, r.y, r.z, 0,
        u.x, u.y, u.z, 0,
        f.x, f.y, f.z, 0,
        0, 0, 0, 1
    );

    Matrix scale = Matrix::CreateScale(0.6f, 0.6f, 1.8f);
    Matrix initRot = Matrix::CreateRotationY(DirectX::XMConvertToRadians(0.0f));
    Matrix trans = Matrix::CreateTranslation(m_position);

    Matrix world = scale * initRot * orientation * trans;

    m_bulletModel->Draw(context,states, world, view, proj);

    //-----------------------------------------------------------
    // デバッグコリジョン
    //-----------------------------------------------------------

    if (m_collision && m_displayCollision)
    {
        BoundingBox box;
        box.Center = m_collision->GetCenter();
        box.Extents = m_collision->GetExtents();

        m_displayCollision->AddBoundingBox(box, DirectX::Colors::Red);
    }

}

bool Bullet::IsDead() const
{
    return (!m_isAlive || m_lifetime >= MAX_LIFETIME);
}

void Bullet::BulletKill()
{
    m_isAlive = false;
}

BoundingBox Bullet::GetBoundingBox() const
{
    BoundingBox box;
    if (m_collision)
    {
        box.Center = m_collision->GetCenter();
        box.Extents = m_collision->GetExtents();
    }
    else
    {
        box.Center = m_position;
        box.Extents = Vector3(1.0f, 1.0f, 1.0f);
    }
    return box;
}