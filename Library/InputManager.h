
//入力関連をまとめて管理するマネージャー

/////////////////////////////////////////////////////////
#pragma once

#include <Keyboard.h>
#include <Mouse.h>


/// <summary>
/// ゲーム全体のキーボード＆マウスの入力を更新・管理するマネージャークラス
/// </summary>
class InputManager
{
public:

	//キーボード/////////////////////////////////////////////

	//キーボードの現在の状態
	DirectX::Keyboard::State kb = {};

	//キーボードの判定をするトラッカー
	DirectX::Keyboard::KeyboardStateTracker kbTracker;

	/////////////////////////////////////////////////////////

	//マウス/////////////////////////////////////////////////

	//マウスの現在の状態
	DirectX::Mouse::State mouse = {};

	//マウスの判定をするトラッカー
	DirectX::Mouse::ButtonStateTracker mouseTracker;

	//////////////////////////////////////////////////////////

	/// <summary>
	/// キーボードとマウスの最新の入力状態を取得、トラッカーを更新
	/// 毎フレームの最初に必ず呼び出し
	/// </summary>
	void Update()
	{
		//キーボードの状態を取得
		kb = DirectX::Keyboard::Get().GetState();
		
		//押したor離した 更新
		kbTracker.Update(kb);

		//マウスの状態を取得
		mouse = DirectX::Mouse::Get().GetState();

		//押したor離した 更新
		mouseTracker.Update(mouse);

	}

	/// <summary>
	/// 入力状態をリセット
	/// </summary>
	void Reset()
	{
		//キーボードの更新をリセット
		kbTracker.Reset();

		//マウスの更新をリセット
		mouseTracker.Reset();

	}

};
