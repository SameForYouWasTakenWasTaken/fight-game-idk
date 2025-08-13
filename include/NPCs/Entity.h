#pragma once
#include <string>

#include "raylib.h"
#include "spdlog/spdlog.h"

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

	virtual bool CheckCollision(Entity& other);


	// Info functions
	virtual const std::string GetName() const { return m_Name; }
	virtual float GetHp() const { return m_Hp; }
	virtual const Texture2D& GetTexture() const { return m_Texture; }
	virtual void TakeDamage(float damage); // Take damage (Has default functionality)

	virtual Vector2& GetPosition() { return m_Position; }

protected:
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