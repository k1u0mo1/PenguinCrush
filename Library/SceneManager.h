
//シーンを管理するクラス　マネージャー
//////////////////////////////////////////////////////////////
#pragma once

//
#define ESC_QUIT_ENABLE

#ifdef ESC_QUIT_ENABLE
#include "Keyboard.h"
#endif

template <class T>
class SceneManager;

// シーンの基底クラス
template <class T>
class Scene
{
private:

	// シーンマネージャーへのポインタ
	SceneManager<T>* m_sceneManager;

public:

	/// <summary>
	/// コンストラクタ
	/// </summary>
	Scene() : m_sceneManager(nullptr) {}

	/// <summary>
	/// デストラクタ
	/// </summary>
	virtual ~Scene() = default;

	/// <summary>
	/// シーンの初期化
	/// </summary>
	virtual void Initialize() = 0;

	/// <summary>
	/// 毎フレームの更新処理
	/// </summary>
	/// <param name="elapsedTime">前フレームからの経過時間</param>
	virtual void Update(float elapsedTime) = 0;

	/// <summary>
	/// 描画処理
	/// </summary>
	virtual void Render() = 0;

	/// <summary>
	/// 終了処理
	/// </summary>
	virtual void Finalize() = 0;

	/// <summary>
	/// デバイスに依存するリソースを作成する関数
	/// </summary>
	virtual void CreateDeviceDependentResources() {}

	/// <summary>
	/// ウインドウサイズに依存するリソースを作成する関数
	/// </summary>
	virtual void CreateWindowSizeDependentResources() {}

	/// <summary>
	/// デバイスロストした時に呼び出される関数
	/// </summary>
	virtual void OnDeviceLost() {}

public:

	/// <summary>
	/// シーンマネージャーを登録
	/// </summary>
	/// <param name="sceneManager">登録するマネージャーへのポインタ</param>
	void SetSceneManager(SceneManager<T>* sceneManager) { m_sceneManager = sceneManager; }

	// シーンの切り替え関数
	template <class U>
	void ChangeScene();

	/// <summary>
	/// ユーザーが設定したリソース取得関数
	/// </summary>
	/// <returns>m_sceneManager</returns>
	T* GetUserResources();

};

// シーンマネージャークラス
template <class T>
class SceneManager
{
private:

	// 共通でアクセスしたいオブジェクトへのポインタ
	T* m_userResources;

	// 実行中のシーンへのポインタ
	Scene<T>* m_scene;

	// 次のシーンへのポインタ
	Scene<T>* m_nextScene;

	/// <summary>
	/// シーン削除関数
	/// </summary>
	void DeleteScene();

public:

	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="userResources"></param>
	SceneManager(T* userResources = nullptr)
		: m_userResources(userResources)
		, m_scene(nullptr)
		, m_nextScene(nullptr)
	{
	};

	/// <summary>
	/// デストラクタ
	/// </summary>
	virtual ~SceneManager() { DeleteScene(); };

	/// <summary>
	/// 現在のシーンの更新と、予約されたシーンへの切り替え
	/// </summary>
	/// <param name="elapsedTime">前フレームからの経過時間</param>
	void Update(float elapsedTime);

	/// <summary>
	/// 現在のシーンの描画を実行
	/// </summary>
	void Render();

	// デバイスに依存するリソースを作成する関数
	void CreateDeviceDependentResources();

	// ウインドウサイズに依存するリソースを作成する関数
	void CreateWindowSizeDependentResources();

	// デバイスロストした時に呼び出される関数
	virtual void OnDeviceLost();

	/// <summary>
	/// シーンの設定関数
	/// </summary>
	/// <typeparam name="U"></typeparam>
	template <class U>
	void SetScene();

	/// <summary>
	/// 次に遷移するシーンを予約
	/// </summary>
	/// <typeparam name="U">次に遷移したいシーンのクラス型</typeparam>
	template <class U>
	void SetNextScene();

	/// <summary>
	/// ユーザーリソース設定関数
	/// </summary>
	/// <param name="userResources"></param>
	void SetUserResources(T* userResources) { m_userResources = userResources; }

	/// <summary>
	/// ユーザーリソース取得関数
	/// </summary>
	/// <returns>m_userResources</returns>
	T* GetUserResources() { return m_userResources; }

};

// シーンの切り替え関数
template <class T>
template <class U>
void Scene<T>::ChangeScene()
{
	m_sceneManager->SetNextScene<U>();
}

// ユーザーが設定したリソース取得関数
template <class T>
T* Scene<T>::GetUserResources()
{
	assert(m_sceneManager);

	return m_sceneManager->GetUserResources();
}

// シーンの設定関数
template <class T>
template <class U>
void SceneManager<T>::SetScene()
{
	// シーンを削除
	DeleteScene();

	assert(m_scene == nullptr);

	// シーンを生成
	m_scene = new U;

	// シーンにシーンマネージャーへのポインタを設定
	m_scene->SetSceneManager(this);

	// シーンの初期化処理
	m_scene->Initialize();
}

// シーンの設定関数
template <class T>
template <class U>
void SceneManager<T>::SetNextScene()
{
	if (!m_nextScene)
	{
		// シーンを生成
		m_nextScene = new U;
	}
}

// 更新関数
template <class T>
void SceneManager<T>::Update(float elapsedTime)
{
#ifdef ESC_QUIT_ENABLE
	// ESCキーで終了
	auto kb = DirectX::Keyboard::Get().GetState();
	if (kb.Escape) PostQuitMessage(0);
#endif

	// シーンの切り替え処理
	if (m_nextScene)
	{
		DeleteScene();

		assert(m_scene == nullptr);

		// シーンを切り替え
		m_scene = m_nextScene;

		m_nextScene = nullptr;

		// シーンにシーンマネージャーへのポインタを設定
		m_scene->SetSceneManager(this);

		// シーンの初期化処理
		m_scene->Initialize();
	}

	// シーンの更新
	if (m_scene) m_scene->Update(elapsedTime);

}

// 描画関数
template <class T>
void SceneManager<T>::Render()
{
	// シーンの描画
	if (m_scene) m_scene->Render();
}

// デバイスに依存するリソースを作成する関数
template <class T>
void SceneManager<T>::CreateDeviceDependentResources()
{
	if (m_scene) m_scene->CreateDeviceDependentResources();
}

// ウインドウサイズに依存するリソースを作成する関数
template <class T>
void SceneManager<T>::CreateWindowSizeDependentResources()
{
	if (m_scene) m_scene->CreateWindowSizeDependentResources();
}

// デバイスロストした時に呼び出される関数
template <class T>
void SceneManager<T>::OnDeviceLost()
{
	if (m_scene) m_scene->OnDeviceLost();
}

// シーンの削除関数
template <class T>
void SceneManager<T>::DeleteScene()
{
	if (m_scene)
	{
		m_scene->Finalize();

		delete m_scene;

		m_scene = nullptr;
	}
}