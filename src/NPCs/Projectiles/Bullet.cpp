#include "NPcs/Projectiles/Bullet.h"

Bullet::Bullet(float velocity, bool positiveXdirection = false) : 
	Entity("Resources/Projectiles/bullet.png", "Bullet", 1.f),
	m_positiveXdirection(positiveXdirection)
{
	m_Velocity = velocity;

	m_Texture.height /= 2;
	m_Texture.width /= 2;
}

void Bullet::OnUpdate(float dt)
{
	if (m_positiveXdirection)
	{
		m_Position.x -= m_Velocity * dt;
	}
	else
	{
		m_Position.x += m_Velocity * dt;
	}
}

bool Bullet::CheckCollision(Entity& other)
{
	Vector2 otherPosition = other.GetPosition();
	Texture2D otherTexture = other.GetTexture();

	float height = otherTexture.height;
	float width = otherTexture.width;

	if (otherPosition.x + width < m_Position.x)
		return false;
	if (m_Position.x + m_Texture.width < otherPosition.x)
		return false;
	if (otherPosition.y + height < m_Position.y)
		return false;
	if (m_Position.y + m_Texture.height < otherPosition.y)
		return false;

	delete this;
	return true;
}