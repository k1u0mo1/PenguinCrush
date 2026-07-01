
/**
 * @file   ModelManager.h
 * @brief  キャラクターのモデルを一括で管理管理を行うクラス
 * @author 國田知睦
 * @date   2026/07/01
 */

#pragma once
#include "pch.h"
#include <Model.h>
#include <memory>

class ModelManager
{
public:

	/// <summary>
	///	モデルを一括で管理するシングルトンインスタンスを取得
	/// </summary>
	/// <returns>インスタンス</returns>
	static ModelManager* GetInstance()
	{
		static ModelManager instance;
		return &instance;
	}

	/// <summary>
	/// モデルの初期化
	/// </summary>
	/// <param name="device">デバイス</param>
	void Initialize(ID3D11Device* device);


	/// <summary>
	/// モデルのポインタを渡すゲッター
	/// </summary>
	DirectX::Model* GetIdleModel() const { return m_modelIdle.get(); }
	DirectX::Model* GetAttackModel() const { return m_modelAttack.get(); }
	DirectX::Model* GetRushModel() const { return m_modelRush.get(); }
	DirectX::Model* GetDizzyMaterial() const { return m_materialDizzy.get(); }

private:

	ModelManager() = default;

	~ModelManager() = default;

	//実データのモデルの共有ポインタ
	std::shared_ptr<DirectX::Model> m_modelIdle;
	std::shared_ptr<DirectX::Model> m_modelAttack;
	std::shared_ptr<DirectX::Model> m_modelRush;
	std::shared_ptr<DirectX::Model> m_materialDizzy;

};
