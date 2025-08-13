#include "NPCs/Entity.h"

/*
@param texturePath The path to the texture of the entity
@param name The name of the entity
@param hp The health of the entity
*/
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
	if (m_Hp <= 0)
	{
		m_IsAlive = false;
	}
}

void Entity::CommonDraw()
{
	if (this != nullptr)
		DrawTexture(m_Texture, m_Position.x, m_Position.y, WHITE);
}

void Entity::CommonUpdate(float dt)
{
	
}

bool Entity::CheckCollision(std::shared_ptr<Entity> other)
{
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

	spdlog::info("Hit!");
	return true;
}