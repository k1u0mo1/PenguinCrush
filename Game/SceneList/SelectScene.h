
/**
 * @file   SelectScene.h
 * @brief  選択画面の初期化・更新・描画を管理するクラス
 * @author 國田知睦
 * @date   2026/07/01
 */

#pragma once

//使用クラス----------------
#include "pch.h"
#include "UserResources.h"
#include "Library/SceneManager.h"
//--------------------------

//波
#include "Game/GimmickList/WaveManager.h"

//モデル
#include "Model.h"
#include "Effects.h"
#include <CommonStates.h>

#include "Game/SceneList/GamePlayScene.h"

/// <summary>
/// 選択画面の初期化・更新・描画を管理するクラス
/// </summary>
class SelectScene : public Scene<UserResources>
{
private:

	//ステージの配置の半径
	static constexpr float TARGET_RADIUS = 4.0f;
	//非選択時のモデルのスケール
	static constexpr float MODEL_BASE_SCALE = 0.3f;
	//選択時のモデルのスケール
	static constexpr float MODEL_SELECCT_SCALE = 0.5f;
	//UIの描画が開始するX座標
	static constexpr float UI_START_X = 150.0f;
	//UIの描画が開始するY座標
	static constexpr float UI_START_Y = 150.0f;
	//カーソルの位置のオフセット
	static constexpr float CURSOR_OFFSET = 120.0f;
	//カーソルの大きさ
	static constexpr float CURSOR_SCALE = 0.25f;
	//クリアマークのサイズ
	static constexpr float CLEAR_MARK_SCALE = 0.8f;

	//目標の角度の計算
	static constexpr float GOAL_DIRECTION = 1.0f;

	//回転スピード
	static constexpr float ROTATE_SPEED = 5.0f;
	//カメラの画角
	static constexpr float CAMERA_FOV = 45.0f;

	//テクスチャの間隔
	static constexpr float UI_STEP_Y = 60.0f;
	//選択時のテクスチャの大きさ
	static constexpr float TEXTURE_SCALE_SELECTED = 0.8f;
	//非選択時のテクスチャの大きさ
	static constexpr float TEXTURE_SCALE_NORMAL = 0.5f;

	//音量設定
	static constexpr float DEFAULT_BGM_VOLUME = 0.2f;
	static constexpr float DEFAULT_SE_CLICK_VOLUME = 1.0f;
	static constexpr float DEFAULT_SE_VOLUME = 0.2f;

	//カメラ設定
	//カメラの配置の高さ
	static constexpr float CAMERA_EYE_Y = 5.0f;
	//カメラの手前への引き
	static constexpr float CAMERA_EYE_Z = -11.0f;
	//カメラの注視点の高さ
	static constexpr float CAMERA_TARGET_Y = 0.0f;
	//前方のクリップ面
	static constexpr float CAMERA_NEAR = 0.1f;
	//後方のクリップ面
	static constexpr float CAMERA_FAR = 1000.0f;

public:

	//各ステージのクリアフラグ配列
	//シーンを跨いでも消えなくするためのもの
	static inline bool s_isClearedList[(int)GamePlayScene::StageType::COUNT] = { false };

	//継承シーン関数
public:

	/// <summary>
	/// コンストラクタ
	/// </summary>
	SelectScene();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize()override;

	/// <summary>
	/// 更新
	/// </summary>
	/// <param name="elapsedTime">前フレームからの経過時間</param>
	void Update(float elapsedTime)override;

	/// <summary>
	/// 描画
	/// </summary>
	void Render()override;

	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize()override;

	/// <summary>
	/// デバイスに依存するリソースを作成する関数
	/// </summary>
	void CreateDeviceDependentResources() override;

	/// <summary>
	/// ウインドウサイズに依存するリソースを作成する関数
	/// </summary>
	void CreateWindowSizeDependentResources() override;

	/// <summary>
	/// デバイスロストした時に呼び出される関数
	/// </summary>
	void OnDeviceLost() override;

	/// <summary>
	/// 中心を計算してテクスチャを描画
	/// </summary>
	/// <param name="texture">使用テクスチャ</param>
	/// <param name="position">テクスチャの座標</param>
	/// <param name="scale">テクスチャの大きさ</param>
	/// <param name="color">テクスチャの色</param>
	/// <param name="effects">テクスチャを反転させるか</param>
	void DrawTextureCenter(
		ID3D11ShaderResourceView* texture,
		DirectX::SimpleMath::Vector2 position,
		float scale = 1.0f,
		DirectX::XMVECTOR color = DirectX::Colors::White,
		DirectX::SpriteEffects effects = DirectX::SpriteEffects_None
	);

private:

	//ステージ情報をまとめる
	struct  StageData
	{
		//ステージ名
		std::wstring name;
		//テクスチャ画像
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture;
		//３Dモデル
		std::unique_ptr<DirectX::Model> model;
		//ステージタイプ
		GamePlayScene::StageType type = GamePlayScene::StageType::EASY;
	};

	//ステージのリスト
	std::vector<StageData> m_stageList;

	/// <summary>
	/// ステージが解放されているかを判定するヘルパー関数
	/// </summary>
	/// <param name="stageUndex">ステージ番号</param>
	/// <returns>ステージ番号を返す</returns>
	bool IsStageUnlocked(int stageIndex) const;

private:
	//リソース
	DX::DeviceResources* m_deviceResources;

	//ビュー行列
	DirectX::SimpleMath::Matrix m_view;

	//射影行列
	DirectX::SimpleMath::Matrix m_proj;

	//ステージの番号
	int m_currentCursor = 0;

	//波
	std::unique_ptr<WaveManager> m_waveManager;

private:
	
	//２D描画
	std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;
	// 描画ステート
	std::unique_ptr<DirectX::CommonStates> m_states;

	
	//アニメーション用のタイマー
	float m_animationTimer = 0.0f;

	//ステージ切り替え回転
	float m_currentAngle = 0.0f;
	float m_targetAngle = 0.0f;

	//テクスチャ
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_backgroundTexture;
	//ボタンUIテクスチャ
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_textureButtonUI;
	//カーソルUIテクスチャ
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_textureCursor;
	//波のUIテクスチャ
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_textureWaveUI;
	//クリアマークのテクスチャ
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_textureClearMark;

private:
	//シーン移動中のフラグ
	bool m_isChangingScene = false;
};
