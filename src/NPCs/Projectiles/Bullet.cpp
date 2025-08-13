#include <vector>
#include <typeinfo>
#include "NPcs/Projectiles/Bullet.h"

// @param parent The parent of the bullet, from whom it will be shot from
// @param velocity The velocity of the bullet
/**
 * @brief Constructs a Bullet projectile.
 *
 * Initializes a Bullet entity with the projectile texture, sets its velocity,
 * facing direction, and parent (shooter). The sprite texture size is halved
 * to make the bullet smaller.
 *
 * @param parent Pointer to the Entity that spawned this bullet; collisions with this parent are ignored.
 * @param velocity Horizontal movement speed of the bullet (units per second).
 * @param positiveXdirection If true the bullet moves right; if false it moves left. Defaults to false.
 */
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

/**
 * @brief Advances the bullet's position along the X axis based on its velocity and elapsed time.
 *
 * Moves the bullet horizontally by m_Velocity * dt. When m_positiveXdirection is true the
 * bullet's X coordinate is decreased; when false it is increased.
 *
 * @param dt Elapsed time (in seconds) since the last update.
 */
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

/**
 * @brief Tests and resolves a collision between this bullet and another entity.
 *
 * Performs an axis-aligned bounding-box (AABB) collision test using each entity's
 * position and texture dimensions. If a collision is detected, this bullet
 * applies 30 damage to the other entity and then deletes itself.
 *
 * Collisions with the bullet's parent (m_Parent) or with the bullet itself are ignored.
 *
 * @param other Shared pointer to the other entity to test against. Must be non-null.
 * @return true if a collision occurred (damage applied and this bullet deleted); false otherwise.
 *
 * @warning On a detected collision this object deletes itself (calls `delete this`). Callers must not access the bullet after this function returns true.
 */
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

/**
 * @brief Check collision against a list of entities.
 *
 * Iterates the provided entities and invokes CheckCollision on each one,
 * returning immediately upon the first detected collision.
 *
 * @param others Collection of entity shared pointers to test against.
 * @return true If any entity collides with the bullet (collision handlers such as
 *              applying damage and deleting the bullet may occur).
 * @return false If no collisions are detected.
 */
bool Bullet::CheckCollision(std::vector<std::shared_ptr<Entity>> others)
{
	// If the bullet is colliding with its parent (i.e the player), then don't do anything
	for (auto entity : others)
	{
		if (CheckCollision(entity)) return true;
	}
	return false;
}