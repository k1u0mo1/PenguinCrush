//リザルトシーンクラス//////////////////////////////

#pragma once

//使用クラス----------------

#include "pch.h"
#include "UserResources.h"
#include "Library/SceneManager.h"
//--------------------------

//モデル描画用
#include <Model.h>
#include <Effects.h>
#include <CommonStates.h>

//２Dテクスチャ用

#include <SpriteBatch.h>
// テクスチャ読み込み用
#include <WICTextureLoader.h>

//雪
#include "WeatherList/Snow.h"
//雨
#include "WeatherList/Rain.h"



class ResultScene : public Scene<UserResources>
{
	//継承シーン関数
public:

	//コンストラクタ
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

	/*/// <summary>
	/// 勝敗結果を保持するフラグ
	/// trueならクリア画面とBGM、falseならゲームオーバー画面
	/// </summary>
	static bool isClear;*/


private:

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

	//選択番号
	int m_currentCursor = 0;

	//種類
	const int MENU_COUNT = 3;


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

	int m_nextScene = 0;

};

