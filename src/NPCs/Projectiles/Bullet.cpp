#include <vector>
#include <typeinfo>
#include "NPcs/Projectiles/Bullet.h"

// @param parent The parent of the bullet, from whom it will be shot from
// @param velocity The velocity of the bullet
// @param positiveXdirection The direction of the bullet (true = right, false = left)
Bullet::Bullet(Entity* parent, float velocity, bool positiveXdirection = false) : 
	Entity("Resources/Projectiles/bullet.png", "Bullet", 1.f),
	m_positiveXdirection(positiveXdirection),
	m_Parent(parent)
{
	m_Velocity = velocity;

	// Make the bullet a little smaller
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

bool Bullet::CheckCollision(std::shared_ptr<Entity> other)
{
	// If the bullet is colliding with its parent (i.e the player), then don't do anything
	if (m_Parent != nullptr && m_Parent == other.get()) return false;
	if (this == other.get()) return false; // It can't collide with itself
	Vector2 otherPosition = other->GetPosition();
	Texture2D otherTexture = other->GetTexture();

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

	other->TakeDamage(30.f);
	delete this;
	return true;
}

bool Bullet::CheckCollision(std::vector<std::shared_ptr<Entity>> others)
{
	// If the bullet is colliding with its parent (i.e the player), then don't do anything
	for (auto entity : others)
	{
		if (CheckCollision(entity)) return true;
	}
	return false;
}