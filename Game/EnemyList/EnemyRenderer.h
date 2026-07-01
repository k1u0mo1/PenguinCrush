
/**
 * @file   EnemyRenderer.h
 * @brief  敵の描画関連の管理を行うクラス
 * @author 國田知睦
 * @date   2026/07/01
 */

#pragma once
#include "pch.h"
#include <DirectXMath.h>
#include <memory>
#include "Game/Common/DeviceResources.h"
#include "Game/GimmickList/Stage.h"
#include "Game/ShadowRenderer/ShadowRenderer.h"
#include "Game/EnemyList/BossEnemy.h"

class EnemyRenderer
{
private:
    
	//定数系
	//ふらつきエフェクトの回転速度
    static constexpr float DIZZY_SWAY_SPEED = 2.0f; 
	//ふらつき時の揺れる最大角度
    static constexpr float DIZZY_SWAY_ANGLE = 0.2f;  
	//ふらつきエフェクトの高さ
    static constexpr float DIZZY_EFFECT_OFFSET_Y = 2.5f;

    //影の描画パラメータ
    static constexpr float SHADOW_DEFAULT_SCALE = 1.0f;
    static constexpr float SHADOW_DEFAULT_ROT_X = 0.0f;
    static constexpr float SHADOW_DEFAULT_ROT_Z = 0.0f;

public:
    /// <summary>
	/// コンストラクタ
    /// </summary>
    EnemyRenderer();

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~EnemyRenderer() = default;

    /// <summary>
	/// モデルやテクスチャなどのリソースを初期化する
    /// </summary>
    /// <param name="device"></param>
    void Initialize(ID3D11Device* device);

    /// <summary>
	/// モデルを描画する
    /// </summary>
    /// <param name="context">デバイスコンテキスト</param>
    /// <param name="position">敵の位置</param>
    /// <param name="forward">敵の前方向ベクトル</param>
    /// <param name="state">敵の状態</param>
    /// <param name="dizzyRotationY">ふらつきエフェクトの回転角度</param>
    /// <param name="rotationMatrix">回転行列</param>
    /// <param name="view">ビュー行列</param>
    /// <param name="proj">プロジェクション行列</param>
    /// <param name="stage">ステージ情報</param>
    /// <param name="shadowRenderer">影描画用レンダラー</param>
    void Render(
        ID3D11DeviceContext* context,
        const DirectX::SimpleMath::Vector3& position,
        const DirectX::SimpleMath::Vector3& forward,
        EnemyState state, 
        float dizzyRotationY,
        const DirectX::SimpleMath::Matrix& rotationMatrix,
        const DirectX::SimpleMath::Matrix& view,
        const DirectX::SimpleMath::Matrix& proj,
        Stage* stage,
        ShadowRenderer* shadowRenderer
    );

    /// <summary>
    /// 当たり判定用にメインモデルを取得する
    /// </summary>
    /// <returns>メインモデル</returns>
    DirectX::Model* GetMainModel() const { return m_modelIdle; }

    /// <summary>
    /// デバッグ描画用にステートを取得する
    /// </summary>
    /// <returns>ステート</returns> 
    DirectX::CommonStates* GetStates() const { return m_states.get(); }

private:

	//描画用ステート
    std::unique_ptr<DirectX::CommonStates> m_states;

	DirectX::Model* m_modelIdle = nullptr;
    DirectX::Model* m_modelAttack = nullptr;
    DirectX::Model* m_modelRush = nullptr;
    DirectX::Model* m_materialDizzy = nullptr;
    
	//影描画用のテクスチャ
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_shadowTexture;
};

