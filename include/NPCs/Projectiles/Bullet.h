#include <vector>
#include <memory>
#include "NPCs/Entity.h"
#include "NPCs/Player.h"

/**
 * Bullet projectile entity.
 *
 * Represents a projectile spawned by another Entity (the parent). Moves along the X axis
 * in the direction specified at construction and handles collision checks against other entities.
 */

/**
 * Construct a Bullet.
 * @param parent Pointer to the Entity that created this bullet (typically the shooter). May be used to ignore collisions with the creator.
 * @param velocity Initial speed magnitude of the bullet.
 * @param positiveXdirection If true, bullet moves in the positive X direction; otherwise in the negative X direction.
 */

/**
 * Update the bullet state for the frame.
 * @param dt Delta time (seconds) since the last update.
 */

/**
 * Check collision between this bullet and a single other entity.
 * @param other Shared pointer to the other entity to test against.
 * @return true if this bullet collides with the provided entity; false otherwise.
 */

/**
 * Check collisions between this bullet and multiple other entities.
 * @param others Vector of shared pointers to entities to test against.
 * @return true if this bullet collides with any entity in the list; false otherwise.
 */
class Bullet : public Entity
{
public:
	Bullet(Entity* parent, float velocity, bool positiveXdirection);
private:
	Entity* m_Parent;
	bool m_positiveXdirection;
	void OnUpdate(float dt) override;
	bool CheckCollision(std::shared_ptr<Entity> other) override;
	bool CheckCollision(std::vector<std::shared_ptr<Entity>> others) override;
};