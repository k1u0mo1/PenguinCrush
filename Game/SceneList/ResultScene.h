
/**
 * @file   ResultScene.h
 * @brief  リザルト画面の初期化・更新・描画を管理するクラス
 * @author 國田知睦
 * @date   2026/06/29
 */

#pragma once
#include "pch.h"
#include "UserResources.h"
#include "Library/SceneManager.h"

//モデル描画用
#include <Model.h>
#include <Effects.h>
#include <CommonStates.h>

//２Dテクスチャ用
#include <SpriteBatch.h>
// テクスチャ読み込み用
#include <WICTextureLoader.h>

//雪
#include "Game/WeatherList/Snow.h"
//雨
#include "Game/WeatherList/Rain.h"

/// <summary>
/// リザルト画面の初期化・更新・描画を管理するクラス
/// </summary>
class ResultScene : public Scene<UserResources>
{
private:
	//モデルサイズ
	static constexpr float MODEL_SCALE = 1.0f;
	//勝った際の回転スピード
	static constexpr float CLEAR_MODEL_ROTATION_SPEED = 2.0f;
	//負けた時のモデルの描画位置
	static constexpr float OVER_MODEL_HEIGHT = 0.5f;
	//勝った時のジャンプの速さ
	static constexpr float JUMP_SPEED = 10.0f;
	//勝った時のジャンプの高さ
	static constexpr float JUMP_HEIGHT = 1.0f;

	//カメラ座標
	static constexpr float WEATHER_CAMERA_POS_Y =   5.0f;
	static constexpr float WEATHER_CAMERA_POS_Z = -15.0f;

	//画像の高さ
	static constexpr float RESULT_LOGO_POS_Y_RATIO = 0.3f;

	//ボタンの拡大率
	//選択中
	static constexpr float BUTTON_SCALE_SELECTED = 1.2f;
	//選択されてない
	static constexpr float BUTTON_SCALE_NORMAL = 1.0f;
	//ボタンの基準の高さ
	static constexpr float BUTTON_START_Y_RATIO = 0.6f;
	//ボタン同士の間隔
	static constexpr float BUTTON_STEP_Y = 100.0f;

	//カーソルの中心から左右の離れ具合
	static constexpr float CURSOR_OFFSET_X = 150.0f;
	//カーソルの拡大率
	static constexpr float CURSOR_SCALE = 0.25f;

	//効果音の音量サイズ
	static constexpr float DEFAULT_SE_VOLUME = 0.2f;
	static constexpr float DEFAULT_SE_CLICK_VOLUME = 1.0f;
	static constexpr float DEFAULT_BGM_VOLUME = 0.2f;

	//３Dカメラ設定
	//カメラの配置高さ
	static constexpr float CAMERA_EYE_Y = 2.0f;     
	//カメラの手前への引き具合
	static constexpr float CAMERA_EYE_Z = -6.0f;    
	//カメラの注視点の高さ
	static constexpr float CAMERA_TARGET_Y = 1.0f;     
	//カメラの視野角(度)
	static constexpr float CAMERA_FOV = 45.0f;    
	//前方クリップ面
	static constexpr float CAMERA_NEAR = 0.1f;     
	//後方クリップ面
	static constexpr float CAMERA_FAR = 1000.0f;  

private:

	//選択する種類
	enum class MenuType
	{
		Retry=0,
		Select,
		Title,
		Count
	};

	//選択しているタイプを合わせる
	MenuType m_currentCursor = MenuType::Retry;

	//継承シーン関数
public:

	/// <summary>
	/// コンストラクタ
	/// </summary>
	ResultScene();

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
	//デバイスリソース
	DX::DeviceResources* m_deviceResources;
	//ビュー行列
	DirectX::SimpleMath::Matrix m_view;
	//射影行列
	DirectX::SimpleMath::Matrix m_proj;

	//モデルポインタ
	std::unique_ptr<DirectX::Model> m_resultModel;
	std::unique_ptr<DirectX::CommonStates> m_states;

	// アニメーション用タイマー
	float m_animationTimer = 0.0f;

private:
	//スプライトバッチ
	std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;

	//テクスチャ関連
	//クリア
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_textureClear;
	//失敗
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_textureOver;
	//リトライ
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_textureRetry;
	//選択画面
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_textureSelect;
	//タイトルへ
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_textureTitle;

	// ボタンUIテクスチャ
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_textureButtonUI;
	// カーソルUIテクスチャ
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_textureCursor;

	//雪
	std::unique_ptr<Snow> m_snow;
	//雨
	std::unique_ptr<Rain> m_rain;

private:
	//シーン移動中のフラグ
	bool m_isChangingScene = false;
	//ステージ
	int m_nextScene = 0;

};

