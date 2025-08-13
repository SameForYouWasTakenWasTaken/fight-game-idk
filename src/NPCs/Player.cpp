#include "NPCs/Player.h"
#include "NPCs/Projectiles/Bullet.h"
static bool aiming_left = false;

Player::Player()
	: Entity("resources/Player/idle.png", "Player", 300.f)
{ }

void Player::OnUpdate(float dt)
{
	if (IsKeyDown(KEY_W))
	{
		aiming_left = false;
		m_Texture = LoadTexture(UP);
		m_Position.y -= m_Velocity * dt;
	}

	if (IsKeyDown(KEY_S))
	{
		aiming_left = false;
		m_Texture = LoadTexture(IDLE);
		m_Position.y += m_Velocity * dt;
	}

	if (IsKeyDown(KEY_A))
	{
		aiming_left = true;
		m_Texture = LoadTexture(LEFT);
		m_Position.x -= m_Velocity * dt;
	}

	if (IsKeyDown(KEY_D))
	{
		aiming_left = false;
		m_Texture = LoadTexture(RIGHT);
		m_Position.x += m_Velocity * dt;
	}

	if (IsKeyPressed(KEY_F) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
	{
		Bullet* bullet = new Bullet(1000.f, aiming_left);
		bullet->GetPosition() = 
		{
			static_cast<float>(m_Texture.width / 2) + m_Position.x,
			static_cast<float>(m_Texture.height / 2) + m_Position.y
		};
		m_Bullets.push_back(bullet);
	}

	for (auto bullet : m_Bullets)
	{
		const float pos = bullet->GetPosition().x;
		if (pos > 5000 || pos < -5000)
		{
			m_Bullets.erase(std::remove(m_Bullets.begin(), m_Bullets.end(), bullet), m_Bullets.end());
			delete bullet;
		}
	}

	for (auto bullet : m_Bullets)
		bullet->Update(dt);
	spdlog::info("{}", m_Bullets.size());
}
