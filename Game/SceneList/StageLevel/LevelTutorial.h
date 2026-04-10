
//チュートリアル

#pragma once
#include "LevelBase.h"

//テクスチャを出す用
#include "WICTextureLoader.h"
#include "SpriteBatch.h"
#include "CommonStates.h"

/// <summary>　
/// チュートリアルステージの管理とUI表示を行うクラス
/// </summary>
class LevelTutorial :public LevelBase
{
private:

	//表示フラグ
	bool m_isShowHelp = true;

	//操作方法UI
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_textureUI;
	
	//描画用バッチ
	std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;
	//ステート
	std::unique_ptr<DirectX::CommonStates> m_states;

public:

	/// <summary>
	/// チュートリアルレベルの初期化
	/// </summary>
	/// <param name="deviceResources">デバイスリソース</param>
	/// <param name="stageManager">ステージマネージャー</param>
	/// <param name="enemyManager">エネミーマネージャー</param>
	/// <param name="fishManager">魚（ギミック）マネージャー</param>
	/// <param name="displayCollision">当たり判定の表示用</param>
	void Initialize(
		DX::DeviceResources* deviceResources,
		StageManager* stageManager,
		EnemyManager* enemyManager,
		FishManager* fishManager,
		/*GimmickManager* gimmickManager,*/
		std::shared_ptr<DisplayCollision> displayCollision
	)override
	{
		auto device = deviceResources->GetD3DDevice();
		auto context = deviceResources->GetD3DDeviceContext();

		//作成
		m_spriteBatch = std::make_unique<DirectX::SpriteBatch>(context);
		m_states = std::make_unique<DirectX::CommonStates>(device);

		//テクスチャの読み込み
		DirectX::CreateWICTextureFromFile(
			device,
			L"Resources\\Textures\\TutorialUI.png",
			nullptr,
			m_textureUI.GetAddressOf()
		);

		//---------------------------------------
		//  ステージ（地面）の設定
		//---------------------------------------
		stageManager->SetCurrentStage(L"DefaultStage");

		//---------------------------------------
		// 敵の配置
		//---------------------------------------
		// 敵の初期化（リセット）
		enemyManager->Initialize(
			deviceResources,
			stageManager->GetCurrentStage(),
			displayCollision
		);

		//---------------------------------------
		// 魚（ギミック）の配置
		//---------------------------------------

		fishManager->SetStage(stageManager->GetCurrentStage());

	}

	/// <summary>
	/// チュートリアルレベルの更新
	/// </summary>
	/// <param name="dt">前フレームからの経過時間</param>
	/// <param name="resources">ユーザーリソース</param>
	void Update(float /*dt*/, UserResources* resources) override
	{
		auto input = resources->GetInputManager();

		// [2]キーを押すと、表示/非表示を反転させる
		if (input->kbTracker.pressed.D3)
		{
			m_isShowHelp = !m_isShowHelp;
		}
	}

	/// <summary>
	/// チュートリアルレベルの描画
	/// </summary>
	/// <param name="resources">ユーザーリソース</param>
	void Render(UserResources* /*resources*/) override
	{
		//フラグがfalseなら何も描画しない
		if (!m_isShowHelp) return;

		//テクスチャの描画
		if (m_spriteBatch && m_textureUI)
		{
			m_spriteBatch->Begin(
				DirectX::SpriteSortMode_Deferred,
				m_states->NonPremultiplied()
			);

			//描画
			m_spriteBatch->Draw(m_textureUI.Get(), DirectX::SimpleMath::Vector2(0, 0));

			m_spriteBatch->End();

		}

		//auto debugFont = resources->GetDebugFont();

		//// 説明文の描画
		//float x = 900.0f;
		//float y = 200.0f;
		//float line = 30.0f;

		//debugFont->AddString(L"=== TUTORIAL ===", DirectX::SimpleMath::Vector2(x, y));
		//y += line;
		//debugFont->AddString(L"[3] : Toggle Help", DirectX::SimpleMath::Vector2(x, y)); // 切り替えキーの説明
		//y += line;
		//debugFont->AddString(L"[W] [S] : Move", DirectX::SimpleMath::Vector2(x, y));
		//y += line;
		//debugFont->AddString(L"[MouseL] or [MouseR] : Attack", DirectX::SimpleMath::Vector2(x, y));
		//y += line;
		//debugFont->AddString(L"[W] + [Space] : Dash", DirectX::SimpleMath::Vector2(x, y));
		//y += line;
	}


};

