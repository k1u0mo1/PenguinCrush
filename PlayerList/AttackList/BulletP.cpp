// BulletP.cpp

#include "pch.h"
#include "BulletP.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

BulletP::BulletP(
    const Vector3& pos,
    const Vector3& dir,
    std::shared_ptr<DirectX::Model> model,
    DirectX::CommonStates* states,
    const Stage* stage,
    std::shared_ptr<DisplayCollision> displayCollision,
    float speed
)
    : m_pos(pos)
    , m_model(model)
    , m_states(states)
    , m_stage(stage)
    , m_displayCollision(displayCollision)
    , m_speed(speed)
    , m_lifetime(0.0f)
    , m_isDead(false)
{
    //方向ベクトルを正規化
    m_dir = dir;
    m_dir.Normalize();

    //進行方向をセット
    SetForward(m_dir);

    //コリジョンの初期化
    if (m_model)
    {
        m_collision = std::make_unique<ModelCollisionOrientedBox>(m_model.get());
        m_collision->SetCenter(m_pos);
        m_collision->SetExtents(Vector3(0.25f, 0.25f, 0.25f)); // 当たり判定のサイズ
    }
}

void BulletP::Update(float dt)
{
    if (m_isDead) return;

    Vector3 oldPos = m_pos;

    //移動処理
    m_pos += m_dir * m_speed * dt;
    m_lifetime += dt;

    //寿命による消滅チェック
    if (m_lifetime >= MAX_LIFETIME)
    {
        m_isDead = true;
        return;
    }

    //地形（ステージ）との衝突チェック
    if (m_stage)
    {
        float yOld = m_stage->GetGroundHeight(oldPos.x, oldPos.z);
        float yNew = m_stage->GetGroundHeight(m_pos.x, m_pos.z);

        //前フレームと今フレームで地面を突き抜けたか判定
        if ((oldPos.y > yOld && m_pos.y <= yNew) ||
            (oldPos.y < yOld && m_pos.y >= yNew))
        {
            m_isDead = true;
            return;
        }
    }

    //コリジョンの追従更新
    if (m_collision)
    {
        m_collision->SetCenter(m_pos);
    }
}

void BulletP::Render(
    ID3D11DeviceContext* context,
    const Matrix& view,
    const Matrix& proj)
{
    if (m_isDead || !m_model) return;

    //弾の向きの行列を作成
    Vector3 f = m_dir;
    Vector3 up(0, 1, 0);
    //真上や真下を向いたときのエラー対策
    if (fabs(f.y) > 0.999f) { up = Vector3(1, 0, 0); }

    Vector3 r = up.Cross(f); r.Normalize();
    Vector3 u = f.Cross(r);

    Matrix orientation(
        r.x, r.y, r.z, 0,
        u.x, u.y, u.z, 0,
        f.x, f.y, f.z, 0,
        0, 0, 0, 1
    );

    //スケールと初期回転
    Matrix scale = Matrix::CreateScale(0.4f, 0.3f, 0.3f);
    Matrix initRot = Matrix::CreateRotationY(DirectX::XMConvertToRadians(180.0f));
    Matrix trans = Matrix::CreateTranslation(m_pos);

    //ワールド行列の合成
    Matrix world = scale * initRot * orientation * trans;

    //描画
    m_model->Draw(context, *m_states, world, view, proj);

    //デバッグ用コリジョン描画
    if (m_collision && m_displayCollision)
    {
        m_collision->AddDisplayCollision(m_displayCollision.get());
    }
}
