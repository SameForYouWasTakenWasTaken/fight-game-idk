#pragma once
#include <string>
#include <vector>
#include <memory>

#include "raylib.h"
#include "spdlog/spdlog.h"

/**
	 * Construct an Entity with a texture, name, and starting hit points.
	 * @param texturePath Path to the entity's texture asset.
	 * @param name Human-readable name for the entity.
	 * @param hp Initial hit points (health) of the entity.
	 */
	
	/**
	 * Perform the entity's per-frame update.
	 * @param dt Time delta in seconds since the last update.
	 */
	
	/**
	 * Render the entity.
	 */
	
	/**
	 * Test whether this entity collides with another entity.
	 * Override to provide custom collision logic.
	 * @param other Shared pointer to the other entity to test against.
	 * @return true if this entity collides with `other`, otherwise false.
	 */
	
	/**
	 * Test whether this entity collides with any entity in the provided collection.
	 * Returns true as soon as a collision with any element is found.
	 * @param others Collection of shared pointers to entities to test against.
	 * @return true if a collision is found with any entity in `others`, otherwise false.
	 */
	class Entity 
{
public:
	Entity(
		const char* texturePath,
		const std::string name,
		float hp
	);
	void Update(float dt)
	{
		CommonUpdate(dt);
		OnUpdate(dt); // For subclasses
	}
	void Draw()
	{
		CommonDraw();
		OnDraw(); // For subclasses
	}

	virtual bool CheckCollision(std::shared_ptr<Entity> other);
	virtual bool CheckCollision(std::vector<std::shared_ptr<Entity>> others)
	{
		for (auto entity : others)
		{
			if (CheckCollision(entity)) return true;
		}
		return false;
	}

	// Info functions
	virtual const std::string GetName() const { return m_Name; }
	virtual float GetHp() const { return m_Hp; }
	virtual const Texture2D& GetTexture() const { return m_Texture; }
	virtual void TakeDamage(float damage); /**
 * Returns whether the entity is alive.
 *
 * @return true if the entity is alive; false otherwise.
 */
	virtual bool IsAlive() { return m_IsAlive; }

	/**
 * Get a mutable reference to the entity's position.
 *
 * Returns a non-const reference to the internal Vector2 representing this
 * entity's position so callers can read or modify the position directly.
 *
 * @return Reference to the entity's position (Vector2&).
 */
virtual Vector2& GetPosition() { return m_Position; }

protected:
	bool m_IsAlive = true;
	float m_Hp;
	float m_Velocity = 100.f; // default

	std::string m_Name;

	Texture2D m_Texture;
	Vector2 m_Position = { 0, 0 };


	virtual void OnUpdate(float) {} // Custom update function for flexibility for subclasses (No default functionality)
	virtual void OnDraw() {} // Custom draw function for flexibility for subclasses (No default functionality)
private:
	void CommonUpdate(float dt); // Standard update function for all entities 
	void CommonDraw(); // Standard draw function for all entities
};