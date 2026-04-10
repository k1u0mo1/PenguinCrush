
//StageManeger.h
//ステージを後々増やすために

#include "Game/GimmickList/Stage.h"
#include <unordered_map>
#include <string>

#pragma once

/// <summary>
/// ステージを管理するマネージャー
/// </summary>
class StageManager
{
public:

	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="deviceResources">デバイスリソースへのポインタ</param>
	StageManager(DX::DeviceResources* deviceResources);

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="hwnd">ウィンドウハンドル</param>
	/// <param name="width">画面幅</param>
	/// <param name="height">画面高さ</param>
	void Initialize(HWND hwnd, int width, int height);

	/// <summary>
	/// 新しいステージを生成しリストに追加して登録
	/// </summary>
	/// <param name="name">登録するステージの識別名</param>
	/// <param name="hwnd">ウィンドウハンドル</param>
	/// <param name="width">画面幅</param>
	/// <param name="height">画面高さ</param>
	void AddStage(const std::wstring& name, HWND hwnd, int width, int height);

	/// <summary>
	/// ステージの更新
	/// </summary>
	/// <param name="wave">波のポインタ</param>
	void Update(Wave* wave);

	/// <summary>
	/// ステージの描画
	/// </summary>
	/// <param name="context">デバイスコンテキスト</param>
	/// <param name="view">ビュー行列</param>
	/// <param name="proj">射影行列</param>
	/// <param name="displayCollision">当たり判定の表示用オブジェクト</param>
	void Render(ID3D11DeviceContext* context,
		const DirectX::SimpleMath::Matrix& view,
		const DirectX::SimpleMath::Matrix& proj,
		DisplayCollision* displayCollision);

	/// <summary>
	/// 指定した名前のステージに切り替える
	/// </summary>
	/// <param name="name">切り替え先のステージ名</param>
	void ChangeStage(const std::wstring& name);
	
	/// <summary>
	/// 現在のステージを設定
	/// </summary>
	/// <param name="name">切り替え先のステージ名</param>
	void SetCurrentStage(const std::wstring& name);
	
	/// <summary>
	/// 現在アクティブなステージを取得します。
	/// </summary>
	/// <returns>現在のステージのポインタ</returns>
	[[nodiscard]] Stage* GetCurrentStage() const noexcept { return m_currentStage; }

private:

	DX::DeviceResources* m_deviceResources;

	std::unordered_map<std::wstring, std::unique_ptr<Stage>> m_stages;

	Stage* m_currentStage = nullptr;

};

