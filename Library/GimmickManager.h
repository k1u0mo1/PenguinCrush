
//GimmickManager.h
//ギミック全体を管理

#pragma once
#include <vector>
#include <memory>
#include "IGimmick.h"


class GimmickManager
{
public: 

	/// <summary>
	/// 何のギミックの呼び出しか
	/// </summary>
	/// <param name="gimmick"></param>
	void Add(std::shared_ptr<IGimmick> gimmick)
	{
		m_gimmicks.push_back(gimmick);
	}

	/// <summary>
	/// 呼び出されたギミックの初期化
	/// </summary>
	/// <param name="hwnd"></param>
	/// <param name="width"></param>
	/// <param name="height"></param>
	void Initalize(HWND hwnd, int width, int height)
	{
		for (auto& g : m_gimmicks)
		{
			g->Initialize(hwnd, width, height);
		}
	}

	/// <summary>
	/// 呼び出されたギミックの更新
	/// </summary>
	/// <param name="dt"></param>
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
	/// <param name="context"></param>
	/// <param name="view"></param>
	/// <param name="proj"></param>
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

	std::vector<std::shared_ptr<IGimmick>> m_gimmicks;
};

