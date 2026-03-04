

#include "pch.h"
#include "AudioManager.h"

using namespace DirectX;


void AudioManager::Initialize()
{
	//多重初期化を防ぐ
	if (m_audioEngine)
	{
		return;
	}

	AUDIO_ENGINE_FLAGS eflags = AudioEngine_Default;

#ifdef  _DEBUD
	eflags |= AudioEngine_Debug;
#endif //  _DEBUD

	m_audioEngine = std::make_unique<AudioEngine>(eflags);
}

void AudioManager::Update()
{
	
}

void AudioManager::Suspend()
{
	// エンジンを一時停止させる
	if (m_audioEngine)
	{
		m_audioEngine->Suspend();
	}
}

void AudioManager::LoadSound(
	const std::string& key,
	const wchar_t* filename)
{
	//読み込み済みなら何もしない
	if (m_sounds.find(key) != m_sounds.end())
	{
		return;
	}
	//ファイル読み込みをトライ（失敗したらキャッチ）
	try
	{
		auto se = std::make_unique<SoundEffect>(m_audioEngine.get(), filename);
		m_sounds[key] = std::move(se);
	}
	catch (...) // 全ての例外をキャッチ
	{
		// 出力ウィンドウにエラーを表示
		std::string message = "FAILED to load sound file: ";
		// wchar_t* を string に変換して表示するのは手間なので簡易表示
		OutputDebugStringA(message.c_str());
		OutputDebugStringW(filename); // ファイル名を表示
		OutputDebugStringA("\nPlease check the file path!\n");
	}

	//新しく読み込んでリストに追加
	//auto se = std::make_unique<SoundEffect>(m_audioEngine.get(), filename);

	//m_sounds[key] = std::move(se);


}

void AudioManager::Play(const std::string& key)
{
	//リストにその名前の音があるか確認して再生
	if (m_sounds.find(key) == m_sounds.end())
	{
		return;
	}

	m_sounds[key]->Play(m_seVolume, 0.0f, 0.0f);

}

void AudioManager::SetSEVolume(float volume)
{
	//音量の範囲を0.0～1.0
	if (volume < 0.0f)
	{
		volume = 0.0f;
	}
	if (volume > 1.0f)
	{
		volume = 1.0f;
	}

	m_seVolume = volume;

}

void AudioManager::PlayBGM(const std::string& key)
{
	//指定されたキーの音声データがあるか確認
	if (m_sounds.find(key) == m_sounds.end())
	{
		return; 
	}

	//同じBGMだったら継続させる
	if (m_bgmInstance && m_currentBGMKey == key)
	{
		return;
	}


	//すでにBGMが鳴っていたら止める
	StopBGM();

	//インスタンス
	m_bgmInstance = m_sounds[key]->CreateInstance();

	if (m_bgmInstance)
	{
		m_bgmInstance->SetVolume(m_bgmVolume);

		//ループ設定をONにして再生
		m_bgmInstance->Play(true);

		//現在のBGMを記憶
		m_currentBGMKey = key;
	}
}

void AudioManager::StopBGM()
{
	if (m_bgmInstance)
	{
		m_bgmInstance->Stop();
		// メモリ解放
		m_bgmInstance.reset(); 
	}

	//記憶していたBGMをクリアする
	m_currentBGMKey.clear();
}

void AudioManager::SetBGMVolume(float volume)
{
	//音量の範囲を0.0～1.0
	if (volume < 0.0f)
	{
		volume = 0.0f;
	}
	if (volume > 1.0f)
	{
		volume = 1.0f;
	}

	m_bgmVolume = volume;

	//現在BGMが流れていたら反映する
	if (m_bgmInstance)
	{
		m_bgmInstance->SetVolume(m_bgmVolume);
	}
}
