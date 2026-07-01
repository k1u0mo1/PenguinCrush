
/**
 * @file   Stage.h
 * @brief  ゲームの足場（ステージ）を管理・描画するクラス
 * @author 國田知睦
 * @date   2026/07/01
 */

#pragma once
#include "pch.h"
#include "Game/Common/DeviceResources.h"
#include "Game/Common/StepTimer.h"
#include "Effects.h"
#include "Game/Collision/ModelCollision.h"
#include "Game/Collision/DisplayCollision.h"
#include "Game/GimmickList/WaveManager.h"

/// <summary>
/// ゲームの足場（ステージ）を管理・描画するクラス
/// 波の影響を受けて傾くギミック
/// </summary>
class Stage : public DX::IDeviceNotify
{
private:

    //------------------------------------------------------
    //ステージのサイズ関連
    //------------------------------------------------------
    //ステージの半分のサイズ （ここを変更すると描画と当たり判定が自動で変更される）
    static constexpr float STAGE_HALF_SIZE = 2.0f;

    //3Dモデル本来の半分のサイズ
    static constexpr float MODEL_BASE_HALF_SIZE = 5.0f;
    //ステージのX軸スケール　（STAGE_HALF_SIZEに連動して自動で計算）
    static constexpr float STAGE_SCALE_X = STAGE_HALF_SIZE / MODEL_BASE_HALF_SIZE;
    //ステージのZ軸スケール　（STAGE_HALF_SIZEに連動して自動で計算）
    static constexpr float STAGE_SCALE_Z = STAGE_HALF_SIZE / MODEL_BASE_HALF_SIZE;
    //ステージのY軸スケール 厚さと高さ
    static constexpr float STAGE_SCALE_Y = 3.0f;
    //ステージの高さのずれを調整するオフセット
    static constexpr float STAGE_OFFSET_Y = 5.0f;
    
    //------------------------------------------------------
    //ステージの揺れと足場1つ1つの情報関連
    //------------------------------------------------------
    //ステージの揺れの大きさ
    static constexpr float STAGE_ANGLE = 100.0f;
    //流氷の数　縦＊横
    static constexpr int DRIFT_ICE_NUM = 20;
    //流氷同士の間隔
    static constexpr float DRIFT_ICE_SPACING = 3.0f;

	//乗っている時に１秒間当たりの食らうダメージの大きさ
	static constexpr float DAMAGE_PER_SECOND = 50.0f;
    //流氷のHP
    static constexpr float ICE_HP = 100.0f;

	//落下判定のY座標　これを下回ったら落ちたとみなす
    static constexpr float FALL_LIMIT_Y = -10.0f;
	//ステージの端のY座標　これを下回るとステージから落ちたとみなす
    static constexpr float STAGE_BOUNDARY_Y = -4.9f;
	
    //------------------------------------------------------
    //カメラの位置関連
    //------------------------------------------------------
    //真下に延ばすレイ
    static constexpr float RAY_START_HEIGHT = 1000.0f;
    static constexpr float RAY_MIN_HEIGHT = -100.0f;

    //リスポーン地点を探す範囲
    static constexpr float RESPAWN_SEARCH_MIN = -10.0f;
    static constexpr float RESPAWN_SEARCH_MAX = 10.0f; 
    static constexpr float RESPAWN_SEARCH_STEP = 2.0f;

    //カメラの位置
    static constexpr float CAMERA_FOV = 45.0f;
    static constexpr float CAMERA_NEAR = 0.1f;
    static constexpr float CAMERA_FAR = 1000.0f;

    //流氷の落ちる速度
    static constexpr float GRAVITY = 1.8f;
    static constexpr float WAVE_BLEND_FACTOR = 0.2f;
    //小さい揺れ
    static constexpr float SMALL_SHAKING = 0.01f;
    //耐久性が低いときの揺れ
    static constexpr float MINUTE_SHAKING = 0.0003f;

    //微数値
    static constexpr float SLIDE_EPSILON = 0.0001f; 

    //流氷の情報
    struct IceFloe
    {
        //ローカルワールド行列
        DirectX::SimpleMath::Vector3 position;
        //初期位置を記憶する
        DirectX::SimpleMath::Vector3 basePosition;
        //X軸傾き
        float rotateX = 0.0f;
        //Z軸傾き
        float rotateZ = 0.0f;
        DirectX::BoundingOrientedBox obb;
        //波の傾きを正確に表す回転行列
        DirectX::SimpleMath::Matrix rotationMatrix = DirectX::SimpleMath::Matrix::Identity;

        //この流氷は崩れているか？
        bool isFallen = false;
        //落ちる速度
        float fallSpeed = 0.0f;
        //耐久値
        float hp = ICE_HP;
    };

public:

    /// <summary>
    /// コンストラクタ
    /// </summary>
    /// <param name="deviceResources">DirectXのデバイスリソース</param>
    Stage(DX::DeviceResources* deviceResources);

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~Stage();

    /// <summary>
    /// 足場（ステージ）の初期化
    /// </summary>
    /// <param name="window">ウィンドウハンドル</param>
    /// <param name="width">画面の幅</param>
    /// <param name="height">画面の高さ</param>
    /// <param name="mapFilename">ステージマップ名</param>
    void Initialize(HWND window, int width, int height, const std::string& mapFilename);

    /// <summary>
    /// 波の状態を基にステージの傾きなどを更新
    /// </summary>
    /// <param name="wave">計算に利用する波オブジェクトのポインタ</param>
    void Update(float elapsedTime, WaveManager* waveManager);

    /// <summary>
    /// 足場（ステージ）の描画
    /// </summary>
    /// <param name="context">Direct3Dのデバイスコンテキスト</param>
    /// <param name="view">カメラのビュー行列</param>
    /// <param name="proj">カメラのプロジェクション行列</param>
    /// <param name="displayCollision">当たり判定用ポインタ</param>
    void Render(ID3D11DeviceContext* context,
        const DirectX::SimpleMath::Matrix& view,
        const DirectX::SimpleMath::Matrix& proj,
        DisplayCollision* displayCollision
    );

    void OnDeviceLost() override;
    void OnDeviceRestored() override;

    /// <summary>
    /// 座標の取得
    /// </summary>
    /// <returns>足場の座標</returns>
    DirectX::SimpleMath::Vector3 GetPosition() const;

    //ステージの角度を取得するためのやつ----------

    /// <summary>
    /// ステージのX軸に対する回転角を取得
    /// </summary>
    /// <returns>X軸の回転角</returns>
    float GetRotateX() const { return m_rotateX; }

    /// <summary>
    /// ステージのZ軸に対する回転角を取得
    /// </summary>
    /// <returns>Z軸の回転角</returns>
    float GetRotateZ() const { return m_rotateZ; }

    //--------------------------------------------

    //指定座標の高さを返す
    //float GetHeightAt(float x, float z) const { return m_position.y; }

    /// <summary>
    /// OBBのコリジョンを取得
    /// </summary>
    /// <returns>OBBのコリジョンのポインタ</returns>
    ModelCollision* GetCollision() const { return m_stageCollision.get(); }

    /// <summary>
    /// 足場の当たり判定を作る
    /// </summary>
    /// <param name="x">>調べる地点のワールドX座標</param>
    /// <param name="z">>調べる地点のワールドZ座標</param>
    /// <returns>座標における足場のY座標</returns>
    float GetGroundHeight(float x, float z)const;

    /// <summary>
    /// ステージのXとZの大きさを取得
    /// </summary>
    /// <param name="x">判定するワールドX座標</param>
    /// <param name="z">判定するワールドZ座標</param>
    /// <returns>指定座標がステージ内にあればtrue、外ならfalse</returns>
    bool IsInside(float x, float z) const;

    /// <summary>
    /// 足場のモデルを取得
    /// ライトで使用するため
    /// </summary>
    /// <returns>足場の3Dモデルのポインタ</returns>
    DirectX::Model* GetModel() const { return m_stageModel.get(); }

    //ステージの傾き処理を渡す

    /// <summary>
    /// 現在のステージの傾きに基づいて、プレイヤー等が滑る方向を取得
    /// </summary>
    /// <returns>滑る方向を示す正規化されたベクトル</returns>
    DirectX::SimpleMath::Vector3 GetSlideDirection(float x, float z) const;

    /// <summary>
    /// 現在のステージの面の向きを取得
    /// </summary>
    /// <returns>上を向いた法線ベクトル</returns>
    DirectX::SimpleMath::Vector3 GetNormal() const;

    /// <summary>
    /// 流氷の個別足場とオブジェの判定を調べる
    /// </summary>
    /// <param name="elapsedTime">前フレームからの経過時間</param>
    /// <param name="x">X座標</param>
    /// <param name="z">Z座標</param>
    void ApplyDamegeToFloe(float elapsedTime, float x, float z);

    /// <summary>
	/// ステージ上の安全なリスポーン位置を見つける
    /// </summary>
    /// <param name="respawnHeight">リスポーン位置の高さ</param>
    /// <param name="boudaryY">境界のY座標</param>
    /// <param name="outPos">見つかったリスポーン位置を格納するベクトル</param>
    /// <returns>安全なリスポーン位置が見つかった場合はtrue、見つからなかった場合はfalse</returns>
    bool FindSafeRespawnPosition(
        float respawnHeight,
        float boundaryY,
        DirectX::SimpleMath::Vector3& outPos) const;

private:

    /// <summary>
    /// デバイス依存のリソースを作成する
    /// </summary>
    void CreateDeviceResources();

    /// <summary>
    /// ウィンドウサイズに依存するリソースを作成・更新する
    /// </summary>
    /// <param name="width">ウィンドウの幅</param>
    /// <param name="height">ウィンドウの高さ</param>
    void CreateWindowSizeResources(int width, int height);

    //デバイスリソース
    DX::DeviceResources* m_deviceResources;

    //射影行列
    DirectX::SimpleMath::Matrix m_proj;
    //ビュー行列
    std::unique_ptr<DirectX::CommonStates> m_states;
    std::unique_ptr<DirectX::Model> m_stageModel;
    
    DirectX::SimpleMath::Vector3 m_position;

    //X軸に対する回転角（ラジアン）
    float m_rotateX;

    //Z軸に対する回転角（ラジアン）
    float m_rotateZ;

    //複数の流氷を管理する配列
    std::vector<IceFloe> m_iceFloes;
    
    std::unique_ptr<DirectX::EffectFactory> m_effectFactory;

    DirectX::SimpleMath::Vector3 stagePointA{ -5.0f,0.0f,0.0f };
    DirectX::SimpleMath::Vector3 stagePointB{ 5.0f,1.0f,0.0f };

    //当たり判定用のオブジェクト
    std::unique_ptr<ModelCollision> m_stageCollision;
    std::unique_ptr<DisplayCollision> m_displayCollision;
    //OBBで使う
    DirectX::BoundingOrientedBox m_localOBB;
    //当たり判定
    bool m_isColliding = false;

    //ステージの大きさを渡す用
    float m_minX, m_maxX;
    float m_minZ, m_maxZ;

};

