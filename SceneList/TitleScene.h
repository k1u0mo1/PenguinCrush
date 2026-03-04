
//タイトルシーンクラス//////////////////////////////

#pragma once

//使用クラス----------------

#include "pch.h"
#include "UserResources.h"
#include "Library/SceneManager.h"
//--------------------------

//雪
#include "WeatherList/Snow.h"
//背景に置くステージ
#include "SceneList/StageLevel/LevelTutorial.h"
//波
#include "GimmickList/Wave.h"

class TitleScene : public Scene<UserResources>
{
//継承シーン関数
public:

	//コンストラクタ
	TitleScene();

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

//描画関数
private:

	//ビュー行列
	DirectX::SimpleMath::Matrix m_view;

	//射影行列
	DirectX::SimpleMath::Matrix m_proj;

//モデルポインタ
private:

	//リソース
	DX::DeviceResources* m_deviceResources;

	//２D描画用
	std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;

	std::unique_ptr<DirectX::CommonStates> m_states;

	
	// テクスチャ
	//タイトル
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_textureTitle; 
	//スタート
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_textureStart; 
	//やめる
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_textureQuit;

	// ボタンUIテクスチャ
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_textureButtonUI;

	// カーソルUIテクスチャ
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_textureCursor;

	//選択カーソル
	int m_currentCursor = 0;

	//背景用
	//雪
	std::unique_ptr<Snow> m_snow;
	//背景ステージ
	std::unique_ptr<Stage> m_backgroundStage;
	//波
	std::unique_ptr<Wave> m_wave;

private:
	//シーン移動中のフラグ
	bool m_isChangingScene = false;

};