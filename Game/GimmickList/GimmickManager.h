
//GimmickManager.h
//ギミック全体を管理

#pragma once
#include <vector>
#include <memory>
#include "IGimmick.h"

/// <summary>
/// ゲーム内のギミックを一括で保持、更新、描画、管理するクラス
/// </summary>
class GimmickManager
{
public: 

	/// <summary>
	/// 管理対象のギミックの追加
	/// </summary>
	/// <param name="gimmick">追加するギミックのポインタ</param>
	void Add(std::shared_ptr<IGimmick> gimmick)
	{
		m_gimmicks.push_back(gimmick);
	}

	/// <summary>
	/// 登録されている全てのギミックを初期化
	/// </summary>
	/// <param name="hwnd">ウィンドウハンドル</param>
	/// <param name="width">画面の幅</param>
	/// <param name="height">画面の高さ</param>
	void Initialize(HWND hwnd, int width, int height)
	{
		for (auto& g : m_gimmicks)
		{
			g->Initialize(hwnd, width, height);
		}
	}

	/// <summary>
	/// 登録されている全てのギミックの状態を更新
	/// </summary>
	/// <param name="dt">前回のフレームからの経過時間</param>
	void Update(float dt)
	{
		for (auto& g : m_gimmicks)
		{
			g->Update(dt);
		}
	}

	/// <summary>
	/// 呼び出されたギミックの描画
	/// </summary>
	/// <param name="context">デバイスコンテキスト</param>
	/// <param name="view">ビュー行列</param>
	/// <param name="proj">プロジェクション行列</param>
	void Render(ID3D11DeviceContext* context,
		const DirectX::SimpleMath::Matrix& view,
		const DirectX::SimpleMath::Matrix& proj)
	{
		for (auto& g : m_gimmicks)
		{
			g->Render(context, view, proj);
		}
	}

	/// <summary>
	/// 登録されているギミックの中から、指定したクラスのギミックを検索して取得
	/// </summary>
	/// <typeparam name="T">取得したいギミックの型</typeparam>
	/// <returns>見つかった場合はそのギミックの共有ポインタ</returns>
	template<typename T>
	std::shared_ptr<T> Get()
	{
		static_assert(std::is_base_of_v<IGimmick, T>, "T must inherit from IGimmick.");

		for (auto& g : m_gimmicks)
		{
			if (auto ptr = std::dynamic_pointer_cast<T>(g))
			{
				return ptr;
			}
		}
		return nullptr;

	}

private:

	//登録されたギミックを保持するリスト
	std::vector<std::shared_ptr<IGimmick>> m_gimmicks;
};

