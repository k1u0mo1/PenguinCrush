
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

	//-----------------------------------------------------
	//アクションの抽象化
	//-----------------------------------------------------

	/// <summary>
	/// キャンセルアクションが入力されたか
	/// </summary>
	/// <returns>入力された場合は true、それ以外は false</returns>
	bool IsCancelAction() const
	{
		//プレイヤーが戻るために押すキー
		return kbTracker.pressed.T;
	}

	/// <summary>
	/// リトライアクションが入力されたか
	/// </summary>
	/// <returns>入力された場合は true、それ以外は false</returns>
	bool IsRetryAction() const
	{
		//プレイヤーがやり直すために押すキー
		return kbTracker.pressed.R;
	}

	/// <summary>
	/// タイトルへ強制的に戻るアクションが入力されたか
	/// </summary>
	/// <returns>入力された場合は true、それ以外は false</returns>
	bool IsForceQuitAction() const
	{
		return kbTracker.pressed.O;
	}

	/// <summary>
	/// 決定アクションが入力されたか
	/// </summary>
	/// <returns>入力された場合は true、それ以外は false</returns>
	bool IsSubmitAction() const
	{
		return kbTracker.pressed.Enter;
	}


};
