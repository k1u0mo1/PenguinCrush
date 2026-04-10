
//アニメーション関連　まだ未実装

#pragma once
#include <string>
#include <vector>
#include <map>
#include <SimpleMath.h>

//ボーン1本当たりの情報
struct BoneInfo
{
	int id;                              //ボーン番号
	std::string name;                    //名前
	DirectX::SimpleMath::Matrix offsets; //初期ポーズの逆行列
	int parentId;						 //親ボーンの番号
};

class Animator
{
public:

	//コンストラクタ
	Animator();

	//毎フレームの更新
	void Update(float elapsedTime);

	//再生するアニメーションを切り替える
	void Play(const std::string& /*animName*/, bool /*loop*/ = true);

	//現在の計算済みボーン行列の配列を取得
	const std::vector<DirectX::SimpleMath::Matrix>& GetFinalMatrices() const
	{
		return m_finalMatrices;
	}


private:

	//内部的な行列の計算
	void CalculateBoneMatrices(int /*boneId*/, const DirectX::SimpleMath::Matrix& /*parentTransform*/);

private:

	//再生中の名前
	std::string m_currentAnimation;

	//現在の再生時間
	float m_currentTime;

	//ループするかどうか？
	bool m_isLooping;

	//最終的にシェーダに渡す行列の配列
	std::vector<DirectX::SimpleMath::Matrix> m_finalMatrices;

};