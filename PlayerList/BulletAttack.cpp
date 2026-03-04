

#include "pch.h"
#include "BulletAttack.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

//コンストラクタ
BulletAttack::BulletAttack(
	const DirectX::SimpleMath::Vector3& pos,
	const DirectX::SimpleMath::Vector3& dir,
	std::shared_ptr<DirectX::Model> model,
	DirectX::CommonStates* states,
	std::shared_ptr<DisplayCollision> displayCollision)
	: m_pos(pos)
	, m_model(model)
	, m_states(states)
	, m_displayCollision(displayCollision)
{
	m_forward = dir;
	m_forward.Normalize();

	//弾の速度
	m_vel = m_forward * 25.0f;

	SetForward(dir);

	//コリジョン
	if (model)
	{
		m_collision = std::make_unique<ModelCollisionOrientedBox>(model.get());
		m_collision->SetCenter(m_pos);
		m_collision->SetExtents(Vector3(0.25f, 0.25f, 0.25f));
	}

}

//更新
void BulletAttack::Update(float dt)
{
	//移動
	m_pos += m_vel * dt;

	//一定距離で消滅
	if (m_pos.Length() > 500.0f)
	{
		m_dead = true;
	}

	if (m_collision)
	{
		m_collision->SetCenter(m_pos);
	}
}

//描画
void BulletAttack::Render(
	ID3D11DeviceContext* context,
	const DirectX::SimpleMath::Matrix& view,
	const DirectX::SimpleMath::Matrix& proj)
{

	if (m_dead || !m_model)
	{
		return;
	}

	// 弾の向き
	DirectX::SimpleMath::Vector3 f = m_vel; // 速度ベクトルを向きに
	f.Normalize();
	DirectX::SimpleMath::Vector3 up(0, 1, 0);
	DirectX::SimpleMath::Vector3 r = up.Cross(f); r.Normalize();
	DirectX::SimpleMath::Vector3 u = f.Cross(r);

	DirectX::SimpleMath::Matrix orientation(
		r.x, r.y, r.z, 0,
		u.x, u.y, u.z, 0,
		f.x, f.y, f.z, 0,
		0, 0, 0, 1
	);

	// スケール調整（モデルの縦横比に合わせて調整）
	DirectX::SimpleMath::Matrix scale = DirectX::SimpleMath::Matrix::CreateScale(0.4f, 0.3f, 0.3f);
	DirectX::SimpleMath::Matrix trans = DirectX::SimpleMath::Matrix::CreateTranslation(m_pos);

	//角度
	Matrix initRot = Matrix::CreateRotationY(DirectX::XMConvertToRadians(180.0f));

	DirectX::SimpleMath::Matrix world = scale * initRot * orientation * trans;

	m_model->Draw(context, *m_states, world, view, proj);

	if (m_collision)
	{
		// 攻撃オブジェクトのコリジョンを DisplayCollision に登録
		m_collision->AddDisplayCollision(m_displayCollision.get());

		// DisplayCollision に登録された情報を描画（色は赤を設定）
		/*m_displayCollision->DrawCollision(
			context, nullptr, view, proj,
			DirectX::Colors::Red, 
			DirectX::Colors::Red, 
			0.5f
		);*/

		//m_displayCollision->AddBoundingBox(box, DirectX::Colors::Red);
	}
}
