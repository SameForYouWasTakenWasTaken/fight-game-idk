#include "NPCs/Entity.h"

Entity::Entity(
	const char* texturePath,
	const std::string name,
	float hp
) : m_Name(name), m_Hp(hp), m_Texture(LoadTexture(texturePath))
{}

void Entity::TakeDamage(float damage)
{
	// Damage can't be negative
	if (damage < 0)
		damage = damage * -1;

	m_Hp -= damage;
	// Health can't be less than 0
	if (m_Hp < 0)
		m_Hp = 0;
}

void Entity::CommonDraw()
{
	DrawTexture(m_Texture, m_Position.x, m_Position.y, WHITE);
}

void Entity::CommonUpdate(float dt)
{
	
}

bool Entity::CheckCollision(Entity& other)
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


	return true;
}