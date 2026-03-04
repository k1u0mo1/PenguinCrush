// Stage.h
#pragma once

#include "pch.h"

#include "Common/DeviceResources.h"
#include "Common/StepTimer.h"

#include "Effects.h"

#include <GimmickList/Wave.h>

#include "Collision/ModelCollision.h"
#include "Collision/DisplayCollision.h"

/// <summary>
/// ゲームの足場（ステージ）を管理・描画するクラス
/// 波の影響を受けて傾くギミック
/// </summary>
class Stage : public DX::IDeviceNotify
{
public:

    Stage(DX::DeviceResources* deviceResources);

    ~Stage();

    /// <summary>
    /// 足場（ステージ）の初期化
    /// </summary>
    /// <param name="window"></param>
    /// <param name="width"></param>
    /// <param name="height"></param>
    void Initialize(HWND window, int width, int height);

    /// <summary>
    /// 波の状態を基にステージの傾きなどを更新
    /// </summary>
    /// <param name="wave">計算に利用する波オブジェクトのポインタ</param>
    void Update(Wave* wave);

    /// <summary>
    /// 足場（ステージ）の描画
    /// </summary>
    /// <param name="context"></param>
    /// <param name="view"></param>
    /// <param name="proj"></param>
    /// <param name="displayCollision">当たり判定用ポインタ</param>
    void Render(ID3D11DeviceContext* context,
        const DirectX::SimpleMath::Matrix& view,
        const DirectX::SimpleMath::Matrix& proj,
        DisplayCollision* displayCollision
    );

    void OnDeviceLost() override;
    void OnDeviceRestored() override;

    DirectX::SimpleMath::Vector3 GetPosition() const;

    //ステージの角度を取得するためのやつ----------

    /// <summary>
    /// ステージのX軸に対する回転角を取得
    /// </summary>
    /// <returns>m_rotateX</returns>
    float GetRotateX() const { return m_rotateX; }

    /// <summary>
    /// ステージのZ軸に対する回転角を取得
    /// </summary>
    /// <returns>m_rotateZ</returns>
    float GetRotateZ() const { return m_rotateZ; }

    //--------------------------------------------

    //指定座標の高さを返す
    //float GetHeightAt(float x, float z) const { return m_position.y; }

    //OBBを取得できるようにする
    ModelCollision* GetCollision() const { return m_stageCollision.get(); }

    /// <summary>
    /// 足場の当たり判定を作る
    /// </summary>
    /// <param name="x"></param>
    /// <param name="z"></param>
    /// <returns></returns>
    float GetGroundHeight(float x, float z)const;

    /// <summary>
    /// ステージのXとZの大きさを取得
    /// </summary>
    /// <param name="x"></param>
    /// <param name="z"></param>
    /// <returns></returns>
    bool IsInside(float x, float z) const;

    //ライトで使う
    DirectX::Model* GetModel() const { return m_stageModel.get(); }

    //ステージの傾き処理を渡す

    /// <summary>
    /// 現在のステージの傾きに基づいて、プレイヤー等が滑る方向を取得
    /// </summary>
    /// <returns>滑る方向を示す正規化されたベクトル</returns>
    DirectX::SimpleMath::Vector3 GetSlideDirection() const;

    //面の向きを渡す用　影で使う

    /// <summary>
    /// 現在のステージの面の向きを取得
    /// </summary>
    /// <returns>上を向いた法線ベクトル</returns>
    DirectX::SimpleMath::Vector3 GetNormal() const;

private:
    void CreateDeviceResources();
    void CreateWindowSizeResources(int width, int height);



    //std::unique_ptr<DX::DeviceResources> m_deviceResources;
    DX::DeviceResources* m_deviceResources;

    //  射影行列
    DirectX::SimpleMath::Matrix m_proj;

    std::unique_ptr<DirectX::CommonStates> m_states;
    std::unique_ptr<DirectX::Model> m_stageModel;
    //std::unique_ptr<Imase::DebugCamera> m_camera;

    

    DirectX::SimpleMath::Vector3 m_position;
    DirectX::SimpleMath::Vector3 m_rotation;

    DirectX::SimpleMath::Vector3 m_anglePosition;

    // 位置
    DirectX::SimpleMath::Vector3 StageAngle;

    // X軸に対する回転角（ラジアン）
    float m_rotateX;

    // Y軸に対する回転角（ラジアン）
    float m_rotateY;

    // Z軸に対する回転角（ラジアン）
    float m_rotateZ;


    std::unique_ptr<DirectX::EffectFactory> m_effectFactory;

    DirectX::SimpleMath::Vector3 stagePointA{ -5.0f,0.0f,0.0f };

    DirectX::SimpleMath::Vector3 stagePointB{ 5.0f,1.0f,0.0f };


    std::unique_ptr<ModelCollision> m_stageCollision;
    std::unique_ptr<DisplayCollision> m_displayCollision;

    //当たり判定
    bool m_isColliding = false;

    //OBBで使う
    DirectX::BoundingOrientedBox m_localOBB;

    //ステージの大きさを渡す用
    float m_minX, m_maxX;
    float m_minZ, m_maxZ;

};

