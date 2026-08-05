/**
 * @file   Stage.cpp
 * @brief  ゲームの足場（ステージ）を管理・描画するクラス
 * @author 國田知睦
 * @date   2026/07/01
 */

#include "pch.h"
#include "Stage.h"
#include <algorithm>
#include <cmath>
#include <fstream>

//----------------------------------------------------------
// コンストラクタ
//----------------------------------------------------------

Stage::Stage(DX::DeviceResources* deviceResources)
    :
    m_deviceResources(deviceResources),
    m_position(0, 0, 0),
    m_rotateX(0.0f),
    m_rotateZ(0.0f),
    m_minX(),
    m_maxX(),
    m_minZ(),
    m_maxZ()
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

void Stage::Initialize(
    HWND window, int width, int height,
    const std::string& mapFilename)
{
    UNREFERENCED_PARAMETER(window);

    CreateDeviceResources();
    CreateWindowSizeResources(width, height);

    //当たり判定見る用
    if (m_stageModel)
    {
        //ModelCollisionOrientedBox を使って、モデルに合わせた箱を作る
        m_stageCollision = std::make_unique<ModelCollisionOrientedBox>(m_stageModel.get());
    }
    //-----------------------------------------------------------

    //ステージ全体の有効範囲を自動計算するための初期値
    float minX = 1e10f, maxX = -1e10f;
    float minZ = 1e10f, maxZ = -1e10f;

    //ペイントでステージを作る//-------------------------------

    //画像ファイルをバイナリモードで開く
    std::ifstream file(mapFilename, std::ios::binary);

    //画像ファイルがあるか　なかったら返す
    if (!file.is_open())
    {
        OutputDebugStringA("画像 StageMap.bmp の読み込みに失敗しました\n");
        return;
    }

    //BMPのヘッダー情報を読み、画像の幅と高さを取得
    unsigned char header[54]{};
    file.read(reinterpret_cast<char*>(header), 54);

    int mapWidth  = *(int*)&header[18];
    int mapHeight = *(int*)&header[22];

    //画像のサイズがおかしい場合は止める
    if (mapWidth <= 0 || mapHeight <= 0)
    {
        OutputDebugStringA("ステージマップ画像のサイズが違う");
        return;
    }

    //BMPの1行のデータサイズは４の倍率になるように調整される仕様の計算
    int rowPadded = (mapWidth * 3 + 3) & (~3);
    unsigned char* rowData = new unsigned char[rowPadded];

    //BMPは画像の下側から保存するため
    for (int y = 0; y < mapHeight; y++)
    {
        file.read(reinterpret_cast<char* > (rowData), rowPadded);

        //実際のゲームの上のZ座標を反転させて計算
        int actualZ = mapHeight - 1 - y;

        for (int x = 0; x < mapWidth; x++)
        {
            //画像サイズがrowPaddedを超えていないか確認
            if ((x * 3 + 2) < rowPadded)
            {
                //B→G→Rの順番に色が格納
                //青
                unsigned char B = rowData[x * 3];
                //緑
                unsigned char G = rowData[x * 3 + 1];
                //赤
                unsigned char R = rowData[x * 3 + 2];

                //ペイント画像の色を判別して流氷を配置する
                //白色＝流氷
                if (R >= 200 && G >= 200 && B >= 200)
                {
                    //流氷のデータを作る
                    IceFloe floe;
                    //配置座標の計算  mapWidthとmapHeightを基準に中心を原点へ
                    float posX = (x - (mapWidth - 1) / 2.0f) * DRIFT_ICE_SPACING;
                    float posZ = (actualZ - (mapHeight - 1) / 2.0f) * DRIFT_ICE_SPACING;

                    //計算した位置を流氷のデータにセットする
                    floe.position = DirectX::SimpleMath::Vector3(posX, 0.0f, posZ);
                    floe.basePosition = floe.position;

                    //各流氷のローカルOBB
                    floe.obb = DirectX::BoundingOrientedBox(
                        DirectX::SimpleMath::Vector3(0, STAGE_OFFSET_Y, 0),
                        DirectX::SimpleMath::Vector3(
                            MODEL_BASE_HALF_SIZE * STAGE_SCALE_X,
                            STAGE_SCALE_Y,
                            MODEL_BASE_HALF_SIZE * STAGE_SCALE_Z
                        ),
                        DirectX::SimpleMath::Quaternion::Identity
                    );

                    m_iceFloes.push_back(floe);

                    //有効範囲の更新
                    if (floe.position.x - MODEL_BASE_HALF_SIZE < minX)minX = floe.position.x - MODEL_BASE_HALF_SIZE;
                    if (floe.position.x + MODEL_BASE_HALF_SIZE > maxX)maxX = floe.position.x + MODEL_BASE_HALF_SIZE;
                    if (floe.position.z - MODEL_BASE_HALF_SIZE < minZ)minZ = floe.position.z - MODEL_BASE_HALF_SIZE;
                    if (floe.position.z + MODEL_BASE_HALF_SIZE > maxZ)maxZ = floe.position.z + MODEL_BASE_HALF_SIZE;
                }
            }
        }
    }
    //使い終わったメモリとファイルを解放
    delete[] rowData;
    file.close();
    //------------------------------------------------------------
    //ステージの有効範囲を保持
    m_minX = minX;
    m_maxX = maxX;
    m_minZ = minZ;
    m_maxZ = maxZ;
}

//----------------------------------------------------------
// 波の状態を基にステージの傾きなどを更新
//----------------------------------------------------------

float Stage::GetGroundHeight(float x, float z) const
{
    // 真上から真下へのレイ
    DirectX::SimpleMath::Vector3 rayOrigin(x, RAY_START_HEIGHT, z);
    DirectX::SimpleMath::Vector3 rayDir(0, -1, 0);
    
    //プレイヤーの落下ライン初期値
    float highestPoint = RAY_MIN_HEIGHT; 

    //すべての流氷に対してレイを飛ばし、乗っている流氷を探す
    for (const auto& floe : m_iceFloes)
    {
        //完全に落下したら流氷の足場判定を外す
        if (floe.position.y<FALL_LIMIT_Y)continue;
        //流氷のOBBをワールド行列で変換
        DirectX::BoundingOrientedBox obb = floe.obb;
        //流氷の回転と位置を反映
        DirectX::SimpleMath::Matrix collisionMatrix =
            floe.rotationMatrix * 
            DirectX::SimpleMath::Matrix::CreateTranslation(floe.position);

        obb.Transform(obb, collisionMatrix);
        //レイとOBBの交差判定
        float dist = 0.0f;

        //その流氷のOBBと交差するか？
        if (obb.Intersects(rayOrigin, rayDir, dist))
        {
            //交差した場合、レイの原点からの距離を計算して、足場の高さを求める
            float hitY = rayOrigin.y - dist;
            //一番高い位置を足場とする (複数の流氷が重なっていた場合の対策)
            if (hitY > highestPoint)
            {
                highestPoint = hitY;
            }
        }
    }
    return highestPoint;
}

//----------------------------------------------------------
// ステージのXとZの大きさを取得
//----------------------------------------------------------

bool Stage::IsInside(float x, float z) const
{
    //ステージの有効範囲内かどうか
    return (x >= m_minX && x <= m_maxX &&
        z >= m_minZ && z <= m_maxZ);
}

//----------------------------------------------------------
// 現在のステージの傾きに基づいて、プレイヤー等が滑る方向を取得
//----------------------------------------------------------

DirectX::SimpleMath::Vector3 Stage::GetSlideDirection(float x, float z) const
{
    UNREFERENCED_PARAMETER(x);
    UNREFERENCED_PARAMETER(z);

    //個別の流氷
    for (const auto& floe : m_iceFloes)
    {
        //流氷の回転行列から法線を計算
        DirectX::SimpleMath::Vector3 normal = 
            DirectX::SimpleMath::Vector3::TransformNormal(DirectX::SimpleMath::Vector3::Up, floe.rotationMatrix);
        normal.Normalize();

        //重力ベクトルと法線から滑る方向を計算
        DirectX::SimpleMath::Vector3 gravity = DirectX::SimpleMath::Vector3::Down;
        DirectX::SimpleMath::Vector3 slideDir = gravity - (gravity.Dot(normal) * normal);

        if (slideDir.LengthSquared() > SLIDE_EPSILON)
        {
            slideDir.Normalize();
            //傾きの角度を計算して滑る強さを決定
            float slopeAngle = acos(std::max(-1.0f, std::min(1.0f, normal.y)));
            float slideStrength = sinf(slopeAngle) * STAGE_ANGLE;
            return slideDir * slideStrength;
        }
    }
    return DirectX::SimpleMath::Vector3::Zero;
}

//----------------------------------------------------------
// 現在のステージの面の向きを取得
//----------------------------------------------------------

DirectX::SimpleMath::Vector3 Stage::GetNormal() const
{
    //回転計算用 Renderでやっていることと同じ
    DirectX::SimpleMath::Matrix rotX = DirectX::SimpleMath::Matrix::CreateRotationX(m_rotateX);
    DirectX::SimpleMath::Matrix rotZ = DirectX::SimpleMath::Matrix::CreateRotationZ(m_rotateZ);
    //合わせる
    DirectX::SimpleMath::Matrix rotation = rotZ * rotX;

    //ステージの回転に合わせて回す
    DirectX::SimpleMath::Vector3 normal = 
        DirectX::SimpleMath::Vector3::TransformNormal(DirectX::SimpleMath::Vector3::Up, rotation);
    normal.Normalize();

    return normal;
}

//----------------------------------------------------------
// 流氷の個別足場とオブジェの判定を調べる
//----------------------------------------------------------

void Stage::ApplyDamegeToFloe(float elapsedTime, float x, float z)
{
    //キャラクターの頭上から真下にレイを飛ばす
    DirectX::SimpleMath::Vector3 rayOrigin(x, RAY_START_HEIGHT, z);
    DirectX::SimpleMath::Vector3 rayDir(0, -1, 0);
    
    //一番高い流氷を探す
    IceFloe* targetFloe = nullptr;
    float highestPoint = RAY_MIN_HEIGHT;

    //全ての流氷から、現在キャラが乗っているものを探す
    for (auto& floe : m_iceFloes)
    {
        //すでに沈んでいるものは省く
        if (floe.isFallen)continue;
        //流氷のOBBをワールド行列で変換
        DirectX::BoundingOrientedBox obb = floe.obb;
        DirectX::SimpleMath::Matrix collisionMatrix =
            floe.rotationMatrix * 
            DirectX::SimpleMath::Matrix::CreateTranslation(floe.position);
        //OBBをワールド行列で変換
        obb.Transform(obb, collisionMatrix);

        float dist = 0.0f;
        //レイとOBBの交差判定
        if (obb.Intersects(rayOrigin, rayDir, dist))
        {
            //交差した時、レイの原点からの距離を計算して足場の高さを求める
            float hitY = rayOrigin.y - dist;
            //一番高い位置を足場とする
            if (hitY > highestPoint)
            {
                highestPoint = hitY;
                targetFloe = &floe;
            }
        }
    }
    //乗っている流氷のHPを減らす
    if (targetFloe)
    {
        targetFloe->hp -= DAMAGE_PER_SECOND * elapsedTime;
    }
}

//----------------------------------------------------------
// ステージ上の安全なリスポーン位置を見つける
//----------------------------------------------------------

bool Stage::FindSafeRespawnPosition(float respawnHeight, float boundaryY, DirectX::SimpleMath::Vector3& outPos) const
{
	//ステージ全体をグリッド状にチェックしていく
    for (float checkX = RESPAWN_SEARCH_MIN; checkX <= RESPAWN_SEARCH_MAX; checkX += RESPAWN_SEARCH_STEP)
    {
        for(float checkZ = RESPAWN_SEARCH_MIN; checkZ <= RESPAWN_SEARCH_MAX; checkZ += RESPAWN_SEARCH_STEP)
        {
            //調べている場所の足場の高さを取得
			float groundY = GetGroundHeight(checkX, checkZ);

			//足場の高さが安全な位置より高いか？
            if (groundY > boundaryY)
            {
				//安全な位置が見つかったので、そこにリスポーンさせる
                outPos = DirectX::SimpleMath::Vector3(checkX, respawnHeight, checkZ);
                return true;
            }
		}
    }
	//安全な位置が見つからなかった
    return false;
}

//----------------------------------------------------------
// リソース
//----------------------------------------------------------

void Stage::CreateDeviceResources()
{
    auto device = m_deviceResources->GetD3DDevice();

    m_states = std::make_unique<DirectX::CommonStates>(device);

    //EffectFactory fx(device);

    m_effectFactory = std::make_unique<DirectX::EffectFactory>(device);
    m_effectFactory->SetDirectory(L"Resources\\Models");
    m_stageModel = DirectX::Model::CreateFromSDKMESH(
        device,
        L"Resources\\Models\\ICENew.sdkmesh",
        *m_effectFactory.get()
    );
}

//----------------------------------------------------------
// 画面リソース
//----------------------------------------------------------

void Stage::CreateWindowSizeResources(int width, int height)
{
    UNREFERENCED_PARAMETER(width);
    UNREFERENCED_PARAMETER(height);

    // 画面サイズの取得
    RECT rect = m_deviceResources->GetOutputSize();

    // 射影行列の作成
    m_proj = DirectX::SimpleMath::Matrix::CreatePerspectiveFieldOfView(
        DirectX::XMConvertToRadians(CAMERA_FOV)
        , static_cast<float>(rect.right) / static_cast<float>(rect.bottom)
        , CAMERA_NEAR, CAMERA_FAR);
}

//----------------------------------------------------------
// 波の状態を基にステージの傾きなどを更新
//----------------------------------------------------------

void Stage::Update(float elapsedTime, WaveManager* waveManager)
{
    
    if (!waveManager)return;

    //中心座標の波の高さと傾きを１度だけ取得
    float centerWaveHeight = waveManager->GetCurrentHeight(m_position.x, m_position.z);
    DirectX::SimpleMath::Vector2 centerSlope = waveManager->GetCurrentWaveAngle(m_position.x, m_position.z);

    //ステージ全体の傾きとしてメンバー変数に保存
    m_rotateX = centerSlope.x;
    m_rotateZ = centerSlope.y;
    //ステージの中心の高さを波に合わせる
    m_position.y = centerWaveHeight;

    //ステージ全体を共通で傾けるための回転行列を１つ作る
    DirectX::SimpleMath::Matrix stageRotation = 
        DirectX::SimpleMath::Matrix::CreateRotationX(m_rotateX) *
        DirectX::SimpleMath::Matrix::CreateRotationZ(m_rotateZ);

    //全ての流氷の座標と回転を計算
    for (auto& floe : m_iceFloes)
    {
        //流氷のHPが０になったら沈む
        if (!floe.isFallen && floe.hp <= 0.0f)
        {
            floe.isFallen = true;
        }

        //流氷が「崩落」なら落ちる
        if (floe.isFallen)
        {
            //重力で下向きに加速
            floe.fallSpeed += GRAVITY * elapsedTime;
            floe.position.y -= floe.fallSpeed * elapsedTime;

            continue;
        }
        //全体が連動する
        DirectX::SimpleMath::Vector3 idealPos = 
            m_position + DirectX::SimpleMath::Vector3::Transform(floe.basePosition, stageRotation);
        
        //実際の波の高さと実際の波の傾斜を取得
        float localWaveY = waveManager->GetCurrentHeight(idealPos.x, idealPos.z);
        DirectX::SimpleMath::Vector2 localSlope = waveManager->GetCurrentWaveAngle(idealPos.x, idealPos.z);

        //揺れる強さ
        float blendFactor = WAVE_BLEND_FACTOR;
        //高さのブレンド
        idealPos.y = (idealPos.y * (1.0f - blendFactor)) + (localWaveY * blendFactor);
        //傾きのブレンド
        float finalRotateX = (m_rotateX * (1.0f - blendFactor)) + (localSlope.x * blendFactor);
        float finalRotateZ = (m_rotateZ * (1.0f - blendFactor)) + (localSlope.y * blendFactor);

        //流氷のHPによって変化する処理
        if (floe.hp < ICE_HP)
        {
            float sinkOffset = (ICE_HP - floe.hp) * SMALL_SHAKING;
            idealPos.y -= sinkOffset;

            //HPが半分以下になったら激しく揺れる
            if (floe.hp < ICE_HP / 2)
            {
                //HPが低くなるほど大きく揺れる
                float shakeStrength = ((ICE_HP / 2) - floe.hp) * MINUTE_SHAKING;

                finalRotateX += ((rand() % 100) / 50.0f - 1.0f) * shakeStrength;
                finalRotateZ += ((rand() % 100) / 50.0f - 1.0f) * shakeStrength;
                idealPos.x += ((rand() % 100) / 50.0f - 1.0f) * shakeStrength * 0.1f;
                idealPos.z += ((rand() % 100) / 50.0f - 1.0f) * shakeStrength * 0.1f;
            }
        }

        //ブレンド結果を流氷に適用
        floe.rotateX = m_rotateX;
        floe.rotateZ = m_rotateZ;
        floe.rotationMatrix = 
            DirectX::SimpleMath::Matrix::CreateRotationX(finalRotateX) *
            DirectX::SimpleMath::Matrix::CreateRotationZ(finalRotateZ);
        floe.position = DirectX::SimpleMath::Vector3(idealPos.x, idealPos.y, idealPos.z);
    }
}

//----------------------------------------------------------
// 足場（ステージ）の描画
//----------------------------------------------------------

void Stage::Render(
    ID3D11DeviceContext* context,
    const DirectX::SimpleMath::Matrix& view,
    const DirectX::SimpleMath::Matrix& proj,
    DisplayCollision* displayCollision
)
{
    UNREFERENCED_PARAMETER(proj);

    //光の描画
    m_stageModel->UpdateEffects([&](DirectX::IEffect* effect)
    {
        auto basicEffect = dynamic_cast<DirectX::BasicEffect*>(effect);
        if (basicEffect)
        {
            basicEffect->EnableDefaultLighting();
        }   
    });

    //流氷の描画ループ
    for (const auto& floe : m_iceFloes)
    {
        //完全に沈んだ流氷はスキップ
        if (floe.position.y < FALL_LIMIT_Y)continue;

        //描画用ワールド行列
        DirectX::SimpleMath::Matrix world =
            DirectX::SimpleMath::Matrix::CreateScale(
            STAGE_SCALE_X, STAGE_SCALE_Y, STAGE_SCALE_Z)
            * floe.rotationMatrix
            * DirectX::SimpleMath::Matrix::CreateTranslation(
                floe.position + DirectX::SimpleMath::Vector3(0.0f, STAGE_OFFSET_Y, 0.0f)
            );

        //流氷を１つずつ描画
        m_stageModel->Draw(context, *m_states.get(), world, view, m_proj);

        //当たり判定の更新・デバック
        if (m_stageCollision && displayCollision)
        {
            DirectX::BoundingOrientedBox currentObb;
            //流氷のOBBをワールド行列で変換
            DirectX::SimpleMath::Matrix collisionMatrix = 
                floe.rotationMatrix * DirectX::SimpleMath::Matrix::CreateTranslation(floe.position);

            floe.obb.Transform(currentObb, collisionMatrix);

            displayCollision->AddBoundingOrientedBox(currentObb, DirectX::Colors::Red);
        }
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
