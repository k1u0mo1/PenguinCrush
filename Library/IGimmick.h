
//IGimmick.h
//ギミックの共通のインターフェース（ルール的なもの）
//

#pragma once
#include <pch.h>

class IGimmick
{
public:

	/// <summary>
	/// 仮想デストラクタ
	/// </summary>
	virtual ~IGimmick() = default;

	/// <summary>
	/// ギミックの初期化（純粋仮想関数）
	/// </summary>
	/// <param name="hwnd">ウィンドウハンドル</param>
	/// <param name="width">画面の幅</param>
	/// <param name="height">画面の高さ</param>
	virtual void Initialize(HWND hwnd, int width, int height) = 0;
	
	/// <summary>
	/// ギミックの更新処理（純粋仮想関数）
	/// </summary>
	/// <param name="elapsedTime">前フレームからの経過時間</param>
	virtual void Update(float elapsedTime) = 0;
	
	/// <summary>
	/// ギミックの描画処理（純粋仮想関数）
	/// </summary>
	/// <param name="context">デバイスコンテキスト</param>
	/// <param name="view">ビュー行列</param>
	/// <param name="proj">射影行列</param>
	virtual void Render(
		ID3D11DeviceContext* context,
		const DirectX::SimpleMath::Matrix& view,
		const DirectX::SimpleMath::Matrix& proj) = 0;
};
