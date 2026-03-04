
//TransitionMask.h

//画面切り替え用

#include "SpriteBatch.h"

#pragma once

/// <summary>
/// 画面切り替えの演出を管理・描画するクラス
/// </summary>
class TransitionMask
{
public:

	/// <summary>
	/// フェード演出の種類
	/// </summary>
	enum  class FadeType
	{
		WIPE //ワイプ演出

		//追加可能
	};

	/// <summary>
	/// マスク描画のリクエスト状態
	/// </summary>
	enum class CreateMaskRequest
	{
		NONE,
		COPY
	};

public:

	/// <summary>
	/// トランジションマスクを初期化
	/// </summary>
	/// <param name="device">デバイス</param>
	/// <param name="context">デバイスコンテキスト</param>
	/// <param name="interval">演出にかかる時間</param>
	TransitionMask(
		ID3D11Device* device,
		ID3D11DeviceContext* context,
		float interval
	);

	/// <summary>
	/// トランジションの進行度を更新
	/// </summary>
	/// <param name="elapsedTime">前フレームからの経過時間</param>
	void Update(float elapsedTime);

	/// <summary>
	/// トランジションマスクを描画
	/// </summary>
	/// <param name="context">デバイスコンテキスト</param>
	/// <param name="states">コモンステート</param>
	/// <param name="texture">マスクに使用するテクスチャ</param>
	/// <param name="rect">描画領域の矩形</param>
	void Draw(
		ID3D11DeviceContext* context,
		DirectX::CommonStates* states,
		ID3D11ShaderResourceView* texture,
		const RECT& rect
	);

	/// <summary>
	/// 画面の演出を始める
	/// </summary>
	void Open();

	/// <summary>
	/// 画面の演出を終わる
	/// </summary>
	void Close();

	/// <summary>
	/// 現在、画面の演出が始まっているかを取得
	/// </summary>
	/// <returns>Open状態なら true　　m_open</returns>
	bool IsOpen() const { return m_open; }

	/// <summary>
	/// 現在、画面の演出が終わっているかを取得
	/// </summary>
	/// <returns>Close状態なら true　！m_open</returns>
	bool IsClose() const { return !m_open; }

	/// <summary>
	/// 内部の進行レートを直接取得
	/// 0.0f～1.0f
	/// </summary>
	/// <returns>m_rate</returns>
	float GetOpenRate() const { return m_rate; }

	/// <summary>
	/// 演出にかかる時間を設定
	/// </summary>
	/// <param name="interval">秒数　m_interval</param>
	void SetInterval(float interval) { m_interval = interval; }

	/// <summary>
	/// マスク生成のリクエスト状態をセット
	/// </summary>
	/// <param name="request">m_request</param>
	void SetCreateMaskRequest(CreateMaskRequest request) { m_request = request; }

	/// <summary>
	/// 現在のマスク生成リクエスト状態を取得
	/// </summary>
	/// <returns>m_request</returns>
	CreateMaskRequest GetCreateMaskRequest() const { return m_request; }

	/// <summary>
	/// 現在のトランジション進行度を取得　
	/// 0.0f ～ 1.0f
	/// Open時 0.0f->1.0f Close時 0.0f->1.0f
	/// </summary>
	/// <returns>進行度合い　 m_rate</returns>
	float GetRate() const
	{
		if (m_open)return 1.0f - m_rate;
		return m_rate;
	}

	/// <summary>
	/// 演出が完了したかどうか
	/// </summary>
	/// <returns>完全に開く、または閉じる場合は true</returns>
	bool IsEnd() const
	{
		if (GetRate() == 1.0f) return true;
		return false;
	}

	/// <summary>
	/// フェード演出の種類を設定
	/// </summary>
	/// <param name="type">設定する FadeType　m_currentFadeType</param>
	void SetFadeType(FadeType type) { m_currentFadeType = type; }

private:

	CreateMaskRequest m_request;

	//割合
	float m_rate;

	//オープンorクローズするまでの時間
	float m_interval;

	//オープンフラグ
	bool m_open;

	//スプライトバッチ
	std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;

	//フェードのタイプをセット
	FadeType m_currentFadeType = FadeType::WIPE;

	struct ConstantBuffer
	{
		float rate; 
		float pad[3];
	};

	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_fade_PS;

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_maskTexture;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_dummyTexture;
};

