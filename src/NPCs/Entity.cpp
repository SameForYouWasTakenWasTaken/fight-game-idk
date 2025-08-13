#include "NPCs/Entity.h"

/**
 * @brief Constructs an Entity with a texture, name, and initial health.
 *
 * Initializes the entity's name and hit points, and loads the texture from the provided path
 * to initialize the entity's texture resource.
 *
 * @param texturePath File path to the texture image used by the entity.
 * @param name Human-readable name for the entity.
 * @param hp Initial health (hit points) for the entity.
 */
Entity::Entity(
	const char* texturePath,
	const std::string name,
	float hp
) : m_Name(name), m_Hp(hp), m_Texture(LoadTexture(texturePath))
{}

/**
 * @brief Applies damage to the entity's health.
 *
 * Reduces the entity's hit points by the given amount. Negative damage values
 * are treated as their absolute value. If health falls to zero or below, the
 * entity's alive state is set to false.
 *
 * @param damage Amount of damage to apply; negative values are converted to positive.
 */
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

/**
 * @brief Draws the entity's texture at its current position.
 *
 * If the entity instance is valid, renders the entity's texture at m_Position using a WHITE tint.
 */
void Entity::CommonDraw()
{
	if (this != nullptr)
		DrawTexture(m_Texture, m_Position.x, m_Position.y, WHITE);
}

/**
 * @brief Per-frame update hook for the entity.
 *
 * This implementation is a no-op. Derived classes should override this method
 * to update entity state using the elapsed time since the last frame.
 *
 * @param dt Time elapsed since the last frame, in seconds.
 */
void Entity::CommonUpdate(float dt)
{
	
}

/**
 * @brief Tests axis-aligned bounding-box collision between this entity and another.
 *
 * Determines whether this entity's rectangular bounds (position + its texture width/height)
 * overlap the other's rectangular bounds. The function returns false if `other` refers to
 * the same object as this entity or if the boxes are separated on any axis; it returns true
 * when an overlap (collision) is detected.
 *
 * @param other Shared pointer to the other Entity to test for collision; must be non-null.
 * @return true if the entities' bounding boxes overlap (collision detected).
 * @return false if `other` is the same object as this entity or if no overlap is found.
 *
 * Side effects: logs "Hit!" via spdlog when a collision is detected.
 */
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