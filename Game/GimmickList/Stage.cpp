//
// Stage.cpp
//

#include "pch.h"
#include "Stage.h"

#include <algorithm>
#include <cmath>

using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

//----------------------------------------------------------
// コンストラクタ
//----------------------------------------------------------

Stage::Stage(DX::DeviceResources* deviceResources)
    : m_deviceResources(deviceResources)
    , m_position(0, 0, 0)
    , m_rotation(0, 0, 0)

    , m_rotateX(0.0f)
    , m_rotateY(0.0f)
    , m_rotateZ(0.0f)
    , m_minX()
    , m_maxX()
    , m_minZ()
    , m_maxZ()
{

    m_deviceResources->RegisterDeviceNotify(this);
}

//----------------------------------------------------------
// デストラクタ
//----------------------------------------------------------

Stage::~Stage()
{
    //  登録解除
    if (m_deviceResources)
    {
        m_deviceResources->RegisterDeviceNotify(nullptr);
    }
}

//----------------------------------------------------------
// 足場（ステージ）の初期化
//----------------------------------------------------------

void Stage::Initialize(HWND /*window*/, int width, int height)
{
    CreateDeviceResources();

    CreateWindowSizeResources(width, height);

    //ステージ大きさに合わせて範囲を指定させる
    float halfSize = STAGE_HALF_SIZE;

    //幅
    m_minX = -halfSize;
    m_maxX = halfSize;
    m_minZ = -halfSize;
    m_maxZ = halfSize;


    m_stageCollision = 
        ModelCollisionFactory::CreateCollision(
        ModelCollision::CollisionType::OBB,
        m_stageModel.get()
    );

    //デバック表示用DisplayCollision
    auto device = m_deviceResources->GetD3DDevice();
    auto context = m_deviceResources->GetD3DDeviceContext();

    m_displayCollision = std::make_unique<DisplayCollision>(device, context, true, true, 100);

    //ステージ用にローカルOBBを作る
    m_localOBB = DirectX::BoundingOrientedBox(
        DirectX::SimpleMath::Vector3(0, 0, 0),//中心
        DirectX::SimpleMath::Vector3(15.0f, 1.2f, 15.0f),//大きさ
        DirectX::SimpleMath::Quaternion::Identity
    );

    //当たり判定見る用
    if (m_stageModel)
    {
        // ModelCollisionOrientedBox を使って、モデルに合わせた箱を作る
        m_stageCollision = std::make_unique<ModelCollisionOrientedBox>(m_stageModel.get());
    }

}

//----------------------------------------------------------
// 波の状態を基にステージの傾きなどを更新
//----------------------------------------------------------

float Stage::GetGroundHeight(float x, float z) const
{
    if (!IsInside(x, z))
    {
        // プレイヤーの落下判定ラインより低い値を返す
        return -100.0f;
    }

    if (!m_stageModel)return m_position.y;

    DirectX::BoundingOrientedBox obb = m_localOBB;


    //スケール・回転・位置
    using namespace DirectX::SimpleMath;
    SimpleMath::Matrix scale = SimpleMath::Matrix::CreateScale(10.0f,5.0f,10.0f);//<-ここでサイズ
    SimpleMath::Matrix rotX = SimpleMath::Matrix::CreateRotationX(m_rotateX);//上下
    SimpleMath::Matrix rotZ = SimpleMath::Matrix::CreateRotationZ(m_rotateZ);//左右
    SimpleMath::Matrix rotation = rotZ * rotX;//左右と上下を合体
    SimpleMath::Matrix translation = SimpleMath::Matrix::CreateTranslation(m_position);
    // ワールド行列
    SimpleMath::Matrix world = scale * rotation * translation;

    obb.Transform(obb, world);

    //真上から真下にレイを渡す
    Vector3 rayOrigin(x, 1000.0f, z);
    Vector3 rayDir(0, -1, 0);

    float dist = 0.0f;
    if (obb.Intersects(rayOrigin, rayDir, dist))
    {
        return rayOrigin.y - dist;
    }

    // ステージの上面の高さを返す
    return m_position.y; // モデルの上面Y
}

//----------------------------------------------------------
// ステージのXとZの大きさを取得
//----------------------------------------------------------

bool Stage::IsInside(float x, float z) const
{
    return (x >= m_minX && x <= m_maxX &&
        z >= m_minZ && z <= m_maxZ);
}

//----------------------------------------------------------
// 現在のステージの傾きに基づいて、プレイヤー等が滑る方向を取得
//----------------------------------------------------------

DirectX::SimpleMath::Vector3 Stage::GetSlideDirection() const
{
    //return DirectX::SimpleMath::Vector3(sinf(m_rotateZ), 0, sinf(m_rotateX));

    using namespace DirectX::SimpleMath;

    // ステージの傾きを回転行列として適用
    // m_rotateXとm_rotateZは、Stage::Updateで更新されていると仮定します。
    SimpleMath::Matrix stageRotation =
        SimpleMath::Matrix::CreateRotationZ(m_rotateZ) *
        SimpleMath::Matrix::CreateRotationX(m_rotateX);

    // Y軸上向きベクトルを回転し、傾いたステージの法線ベクトルに変換
    SimpleMath::Vector3 normal =
        SimpleMath::Vector3::TransformNormal(
            SimpleMath::Vector3::Up, stageRotation);
    normal.Normalize();

    // 重力ベクトルを定義 (0, -1, 0)
    SimpleMath::Vector3 gravity = SimpleMath::Vector3::Down;

    // 接している平面方向（滑る方向）を計算
    // 重力ベクトルから、法線方向の成分を引く
    SimpleMath::Vector3 slideDir = gravity - (gravity.Dot(normal) * normal);

    // 十分に傾いている時に滑る
    if (slideDir.LengthSquared() > 0.0001f)
    {
        slideDir.Normalize();

        // 強さを設定
        // acos(normal.y)で水平から傾斜角度を出す
        float slopeAngle = acos(std::max(-1.0f, std::min(1.0f, normal.y)));

        // sinf(slopeAngle)はどれだけ横に滑るか？
        float slideStrength = sinf(slopeAngle) * 500.0f; // プロトタイプの数値 (50.0f)

        // 速度ベクトル（方向 * 強さ）を返す
        return slideDir * slideStrength;
    }

    // 傾斜が不十分な場合は、ゼロベクトルを返す
    return SimpleMath::Vector3::Zero;
}

//----------------------------------------------------------
// 現在のステージの面の向きを取得
//----------------------------------------------------------

DirectX::SimpleMath::Vector3 Stage::GetNormal() const
{
    //回転計算用 Renderでやっていることと同じ
    SimpleMath::Matrix rotX = SimpleMath::Matrix::CreateRotationX(m_rotateX);
    SimpleMath::Matrix rotZ = SimpleMath::Matrix::CreateRotationZ(m_rotateZ);

    //合わせる
    SimpleMath::Matrix rotation = rotZ * rotX;

    //ステージの回転に合わせて回す
    SimpleMath::Vector3 normal = SimpleMath::Vector3::TransformNormal(SimpleMath::Vector3::Up, rotation);

    normal.Normalize();

    return normal;
}

//----------------------------------------------------------
// リソース
//----------------------------------------------------------

void Stage::CreateDeviceResources()
{
    auto device = m_deviceResources->GetD3DDevice();

    m_states = std::make_unique<CommonStates>(device);

    //EffectFactory fx(device);

    m_effectFactory = std::make_unique<DirectX::EffectFactory>(device);
    m_effectFactory->SetDirectory(L"Resources\\Models");
    //fx.SetDirectory(L"Resources\\Models");
    m_stageModel = Model::CreateFromSDKMESH(
        device,
        L"Resources\\Models\\ICENew.sdkmesh",
        *m_effectFactory.get()
    );
}

//----------------------------------------------------------
// 画面リソース
//----------------------------------------------------------

void Stage::CreateWindowSizeResources(int /*width*/, int /*height*/)
{
    // Optional: implement view-dependent resources
    // 画面サイズの取得
    RECT rect = m_deviceResources->GetOutputSize();

    // 射影行列の作成
    m_proj = SimpleMath::Matrix::CreatePerspectiveFieldOfView(
        XMConvertToRadians(45.0f)
        , static_cast<float>(rect.right) / static_cast<float>(rect.bottom)
        , 0.1f, 1000.0f);
}

//----------------------------------------------------------
// 波の状態を基にステージの傾きなどを更新
//----------------------------------------------------------

void Stage::Update(Wave* wave)
{
    if (!wave) return;

    ////波の高さを加える
    //float waveY = wave->GetHeight(m_position.x, m_position.z);
    //m_position.y =  waveY;

    ////ステージの位置で波の角度を取得
    //Vector2 angles = wave->GetWaveAngle(m_position.x, m_position.z);

    ///*m_rotateX = angles.x;
    //m_rotateZ = angles.y;*/

    ////揺れを優しくしたやつ
    //m_rotateX = angles.x * 0.3f;
    //m_rotateZ = angles.y * 0.3f;

    //////ステージ＆プレイヤーの揺れを消したいとき
    ///*m_rotateX =0;
    //m_rotateZ =0;*/

    //---------------------------------------------------------
    //足場の端の実際のワールド座標での長さを計算
    //---------------------------------------------------------

    float halfWidth = STAGE_HALF_SIZE;
    float halfDepth = STAGE_HALF_SIZE;

    //---------------------------------------------------------
    //足場の前後左右の波の高さ（Y軸）を取得
    //---------------------------------------------------------

    //
    float leftY  = wave->GetHeight(m_position.x - halfWidth, m_position.z);
    //
    float rightY = wave->GetHeight(m_position.x + halfWidth, m_position.z);
    //
    float backY  = wave->GetHeight(m_position.x, m_position.z - halfDepth);
    //
    float frontY = wave->GetHeight(m_position.x, m_position.z + halfDepth);

    //---------------------------------------------------------
    //両端の高低差から実際の傾き角度を出す
    // 
    //角度（ラジアン）=atan2(高さの差, 横幅の距離）
    //---------------------------------------------------------

    //足場の実際の横幅
    float widthDistance = halfWidth * 2.0f;
    //足場の実際の奥行き
    float depthDistance = halfDepth * 2.0f;

    //Z軸回転　左右の傾き
    m_rotateZ = atan2f(rightY - leftY, widthDistance);

    //X軸回転　前後の傾き
    m_rotateX = atan2f(frontY - backY, depthDistance);

}

//----------------------------------------------------------
// 足場（ステージ）の描画
//----------------------------------------------------------

void Stage::Render(
    ID3D11DeviceContext* context,
    const SimpleMath::Matrix& view,
    const SimpleMath::Matrix& proj,
    DisplayCollision* displayCollision
)
{
    if (!m_stageModel)
    {
        return;
    }
    
     //スケール//10.0f, 0.5f, 10.0f
    SimpleMath::Matrix scale = SimpleMath::Matrix::CreateScale(STAGE_SCALE_X, STAGE_SCALE_Y, STAGE_SCALE_Z);
    
    SimpleMath::Matrix rotX = SimpleMath::Matrix::CreateRotationX(m_rotateX);//上下
    SimpleMath::Matrix rotZ = SimpleMath::Matrix::CreateRotationZ(m_rotateZ);//左右
    
    SimpleMath::Matrix rotation = rotZ * rotX;//左右と上下を合体
    SimpleMath::Matrix translation = SimpleMath::Matrix::CreateTranslation(m_position);
    // ワールド行列
    SimpleMath::Matrix world = scale * rotation * translation;
   
    //ステージ描画
    m_stageModel->Draw(context, *m_states.get(), world, view, m_proj);

    if (m_stageCollision && displayCollision)
    {
        // 位置の更新
        m_stageCollision->UpdateBoundingInfo(world);

        // リストに追加
        m_stageCollision->AddDisplayCollision(displayCollision);

        // 奥行き無視で手前に描く
        context->OMSetDepthStencilState(m_states->DepthNone(), 0); 

        displayCollision->DrawCollision(
            context,
            m_states.get(),
            view,
            proj,
            Colors::Yellow, // 線：黄色
            Colors::Red,    // 面：赤
            1.5f            // 不透明度：少し濃くする
        );

        // 描画が終わったら深度設定を元に戻す
        context->OMSetDepthStencilState(m_states->DepthDefault(), 0);
    }
}

void Stage::OnDeviceLost() {}
void Stage::OnDeviceRestored() {}

//----------------------------------------------------------
// 座標の取得
//----------------------------------------------------------

DirectX::SimpleMath::Vector3 Stage::GetPosition() const
{
    return m_position;
}




