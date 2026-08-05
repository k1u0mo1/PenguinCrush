
/**
 * @file   FishManager.h
 * @brief  魚の描画・管理を行うマネージャークラス
 * @author 國田知睦
 * @date   2026/07/16
 */

#pragma once
#include "pch.h"
#include <Game/Common/DeviceResources.h>
#include <Game/Collision/DisplayCollision.h>
#include "Game/GimmickList/Stage.h"
#include "Game/GimmickList/Fish.h"

class ShadowRenderer;
class Player;
class Particle;

class FishManager
{
private:

    //------------------------------------------------------
    //スポーン＆回復関連
    //------------------------------------------------------

    //魚がスポーンする間隔
    static constexpr float FISH_SPAWN_INTERVAL = 1.0f;
    //魚を取得した際のプレイヤーのHP回復量
    static constexpr float FISH_HEAL_AMOUNT = 20.0f;

    //回復した時の音量
    static constexpr float HEAL_SE_VOLUME = 2.0f;

    //エフェクトの高さ
    static constexpr float HIT_EFFECT_HEIGHT = 1.0f;
    //エフェクトの生成数
    static constexpr int HIT_EFFECT_COUNT = 20;
    //エフェクトのサイズ
    static constexpr float HIT_EFFECT_SIZE = 0.2f;
    //------------------------------------------------------
    //スポーン範囲関連
    //------------------------------------------------------

    //スポーン範囲のサイズ
    static constexpr int SPAWN_AREA_SIZE = 100;
    //スポーン範囲　中心
    static constexpr int SPAWN_AREA_OFFSET = 50;
    //スポーン時の高さ
    static constexpr float SPAWN_HEIGHT_OFFSET = 5.0f;

    //魚の初期向き
    static constexpr float DEFAULT_DIR_X = 0.001f;
    static constexpr float DEFAULT_DIR_Y = -1.0f;
    static constexpr float DEFAULT_DIR_Z = 0.0f;

public:

    /// <summary>
    /// FishManagerのインスタンスを生成
    /// </summary>
    /// <param name="deviceResources">デバイスリソース</param>
    /// <param name="displayCollision">表示用の当たり判定オブジェクト</param>
    /// <param name="stage">配置先のステージポインタ</param>
    FishManager(
        DX::DeviceResources* deviceResources,
        std::shared_ptr<DisplayCollision> displayCollision,
        Stage* stage);

    /// <summary>
    /// 魚のスポーンタイマーの更新とプレイヤーとの当たり判定
    /// </summary>
    /// <param name="elapsedTime">前フレームからの経過時間</param>
    /// <param name="player">当たり判定を行うプレイヤーのポインタ</param>
    /// <param name="particle">ヒット時のエフェクト発生用マネージャー</param>
    void Update(float elapsedTime, Player* player, Particle* particle);

    /// <summary>
    /// 存在しているすべての魚の描画
    /// </summary>
    /// <param name="context">デバイスコンテキスト</param>
    /// <param name="view">ビュー行列</param>
    /// <param name="proj">プロジェクション行列</param>
    /// <param name="shadowRenderer">影の描画クラスのポインタ</param>
    void Render(ID3D11DeviceContext* context,
        const DirectX::SimpleMath::Matrix& view,
        const DirectX::SimpleMath::Matrix& proj,
        ShadowRenderer* shadowRenderer);

    /// <summary>
    /// 指定した数の魚をステージ上にランダムな位置にスポーン
    /// </summary>
    /// <param name="count">スポーンさせる魚の数</param>
    void SpawnRandomFish(int count);

    /// <summary>
    /// 参照するステージ情報を更新
    /// </summary>
    /// <param name="stage">新しく参照するステージのポインタ</param>
    void SetStage(Stage* stage) { m_stage = stage; }

private:

    /// <summary>
    /// 魚のスポーン
    /// </summary>
    void SpawnFish();

private:
    //デバイスリソース
    DX::DeviceResources* m_deviceResources;
    //当たり判定の表示用
    std::shared_ptr<DisplayCollision> m_displayCollision;
    Stage* m_stage;

    float m_spawnTimer = 0.0f;
    std::vector<std::unique_ptr<Fish>> m_fish;
};