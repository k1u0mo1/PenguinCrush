
//デバッグ用のフォントクラス

#pragma once

#include <vector>
#include <string>
#include "SimpleMath.h"
#include "CommonStates.h"
#include "Effects.h"
#include "SpriteBatch.h"
#include "SpriteFont.h"

/// <summary>
/// デバック用に２D画面に表示させるためのフォントクラス
/// </summary>
class DebugFont
{
private:

	//文字列情報
	struct String
	{
		//位置
		DirectX::SimpleMath::Vector2 pos;

		//文字列
		std::wstring string;

		//色
		DirectX::SimpleMath::Color color;

		//スケール
		float scale = 1.0f;
	};

	//表示文字列の配列
	std::vector<String> m_strings;

protected:
	//スプライトバッチ
	std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;

	// スプライトフォント
	std::unique_ptr<DirectX::SpriteFont> m_spriteFont;

	// フォントの縦サイズ
	float m_fontHeight;

public:

	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="device">デバイス</param>
	/// <param name="context">デバイスコンテキスト</param>
	/// <param name="fileName">使用するフォントファイル</param>
	DebugFont(ID3D11Device* device, ID3D11DeviceContext* context, wchar_t const* fileName);

	/// <summary>
	/// デストラクタ
	/// </summary>
	virtual ~DebugFont();

	/// <summary>
	/// 描画する2D文字列をキューに登録
	/// </summary>
	/// <param name="string">表示する文字列</param>
	/// <param name="pos">スクリーン座標</param>
	/// <param name="color">描画色</param>
	/// <param name="scale">拡大率</param>
	void AddString(
		const wchar_t* string,
		DirectX::SimpleMath::Vector2 pos,
		DirectX::FXMVECTOR color = DirectX::Colors::White,
		float scale = 1.0f);

	/// <summary>
	/// 登録された2D文字列を一括で描画
	/// </summary>
	/// <param name="states">共通のレンダーステート</param>
	void Render(DirectX::CommonStates* states);

	/// <summary>
	/// フォントの高さを取得する関数
	/// </summary>
	/// <returns>フォントの高さ</returns>
	float GetFontHeight() const { return m_fontHeight; }
};

/// <summary>
/// デバック用に３D画面に表示させるためのフォントクラス
/// </summary>
class DebugFont3D : protected DebugFont
{
private:

	// 文字列情報
	struct String
	{
		// 位置
		DirectX::SimpleMath::Vector3 pos;

		// 文字列
		std::wstring string;

		// 色
		DirectX::SimpleMath::Color color;

		// スケール
		float scale = 1.0f;
	};

	// 表示文字列の配列
	std::vector<String> m_strings;

	// エフェクト
	std::unique_ptr<DirectX::BasicEffect> m_effect;

	// 入力レイアウト
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;

public:

	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="device">デバイス</param>
	/// <param name="context">デバイスコンテキスト</param>
	/// <param name="fileName">使用するフォントファイル</param>
	DebugFont3D(
		ID3D11Device* device,
		ID3D11DeviceContext* context,
		wchar_t const* fileName);

	/// <summary>
	/// デストラクタ
	/// </summary>
	~DebugFont3D();

	/// <summary>
	/// 描画する3D文字列をキューに登録
	/// </summary>
	/// <param name="string">表示する文字列</param>
	/// <param name="pos">3D空間内の座標</param>
	/// <param name="color">描画色</param>
	/// <param name="scale">3D空間内でのサイズ調整用スケール</param>
	void AddString(
		const wchar_t* string,
		DirectX::SimpleMath::Vector3 pos,
		DirectX::FXMVECTOR color = DirectX::Colors::White,
		float scale = 1.0f);

	/// <summary>
	/// 登録された文字列を3D空間で描画
	/// </summary>
	/// <param name="context">デバイスコンテキスト</param>
	/// <param name="states">レンダーステート</param>
	/// <param name="view">ビュー行列</param>
	/// <param name="proj">射影行列</param>
	void Render(
		ID3D11DeviceContext* context,
		DirectX::CommonStates* states,
		const DirectX::SimpleMath::Matrix& view,
		const DirectX::SimpleMath::Matrix& proj);

	/// <summary>
	/// フォントの高さを取得する関数
	/// </summary>
	/// <returns>フォントの高さ　m_fontHeight</returns>
	float GetFontHeight() { return m_fontHeight; }
};

