
//効果音マネージャー

#pragma once
#include <string>
#include <map>
#include <memory>
#include <Audio.h>

/// <summary>
/// ゲーム全体の音声（BGM・SE）を管理・再生するシングルトンクラス
/// </summary>
class AudioManager
{
public:

	/// <summary>
	/// AudioManagerの単一インスタンスを取得
	/// </summary>
	/// <returns>AudioManagerのポインタ</returns>
	static AudioManager* GetInstance()
	{
		static AudioManager instance;

		return &instance;
	}

	/// <summary>
	/// オーディオエンジンの初期化
	/// </summary>
	void Initialize();
	
	/// <summary>
	/// オーディオエンジンの毎フレームの更新
	/// </summary>
	void Update();

	/// <summary>
	/// 音声の再生を一時停止
	/// </summary>
	void Suspend();

	/// <summary>
	/// 音声ファイルを読み込み、指定したキーで登録
	/// </summary>
	/// <param name="key">再生時に指定するための登録名</param>
	/// <param name="filename">読み込む音声ファイルのファイルパス</param>
	void LoadSound(const std::string& key, const wchar_t* filename);

	/// <summary>
	/// 登録済みの効果音（SE）を1回再生
	/// </summary>
	/// <param name="key">再生したい音声の登録名</param>
	void Play(const std::string& key);

	/// <summary>
	/// 効果音（SE）の全体音量を設定
	/// </summary>
	/// <param name="volume">音量（0.0f:無音～1.0f:最大）</param>
	void SetSEVolume(float volume);

	/// <summary>
	/// 登録済みの音声をBGMとしてループ再生
	/// </summary>
	/// <param name="key">再生したいBGMの登録名</param>
	void PlayBGM(const std::string& key);

	/// <summary>
	/// 現在再生中のBGMを停止
	/// </summary>
	void StopBGM();

	/// <summary>
	/// BGMの音量を設定
	/// </summary>
	/// <param name="volume">音量（0.0f:無音～1.0f:最大）</param>
	void SetBGMVolume(float volume);

private:

	AudioManager() {}

	std::unique_ptr<DirectX::AudioEngine> m_audioEngine;

	//文字列
	std::map<std::string, std::unique_ptr<DirectX::SoundEffect>> m_sounds;

	//現在再生中のBGMを保持する
	std::unique_ptr<DirectX::SoundEffectInstance> m_bgmInstance;

	//現在のBGMの音量を記憶する
	float m_bgmVolume = 1.0f;

	//現在のSEの音量を記憶する
	float m_seVolume = 1.0f;

	//BGMを途切れないようにするキー名を記憶する
	std::string m_currentBGMKey;
};

