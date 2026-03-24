
//ステージ選択シーンクラス//////////////////////////////

#pragma once

//使用クラス----------------

#include "pch.h"
#include "UserResources.h"
#include "Library/SceneManager.h"
//--------------------------

//波
#include "GimmickList/Wave.h"

//モデル出すよう
#include "Model.h"
#include "Effects.h"
#include <CommonStates.h>

#include "SceneList/GamePlayScene.h"

class SelectScene : public Scene<UserResources>
{
	//継承シーン関数
public:

	//コンストラクタ
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
		GamePlayScene::StageType type = GamePlayScene::StageType::Stage1;
	};

	std::vector<StageData> m_stageList;

private:
	//リソース
	DX::DeviceResources* m_deviceResources;

	//ビュー行列
	DirectX::SimpleMath::Matrix m_view;

	//射影行列
	DirectX::SimpleMath::Matrix m_proj;

	//ステージの番号
	int m_currentCursor = 0;

	//メニューの数（ステージ数）
	const int MENU_COUNT = 3;

	//波
	std::unique_ptr<Wave> m_wave;

private:
	
	//２D描画
	std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;
	
	// 描画ステート
	std::unique_ptr<DirectX::CommonStates> m_states;

	//// チュートリアル用のモデル
	//std::unique_ptr<DirectX::Model> m_modelTutorial;
	//// ゲーム開始用のモデル
	//std::unique_ptr<DirectX::Model> m_modelGame;

	//std::unique_ptr<DirectX::Model> m_modelGame2;

	// アニメーション用のタイマー
	float m_animationTimer = 0.0f;

	//ステージ切り替え回転
	float m_currentAngle = 0.0f;
	float m_targetAngle = 0.0f;

	// テクスチャ
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_backgroundTexture;

	// ボタンUIテクスチャ
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_textureButtonUI;

	// カーソルUIテクスチャ
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_textureCursor;

private:
	//シーン移動中のフラグ
	bool m_isChangingScene = false;

};
