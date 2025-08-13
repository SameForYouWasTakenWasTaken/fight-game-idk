#pragma once

#include <vector>

#include "NPCs/Entity.h"

#define IDLE "resources/Player/idle.png"
#define LEFT "resources/Player/left.png"
#define RIGHT "resources/Player/right.png"
#define UP "resources/Player/up.png"

/**
 * Player entity representing the user-controlled character.
 *
 * Manages player state, animations, and projectiles spawned by the player.
 */
 
/**
 * Pointers to active bullet entities spawned by the player.
 * The container holds Entity* for each bullet; ownership semantics are determined by the rest of the codebase.
 */
 
/**
 * Construct a Player.
 *
 * Initializes player-specific state and resources.
 */
 
/**
 * Update the player once per frame.
 *
 * @param dt Elapsed time since the last update in seconds.
 */
 
/**
 * Render the player using its current visual state (animation/frame).
 */
class Player : public Entity
{
public:
	std::vector<Entity*> m_Bullets;
	Player();
private:
	void OnUpdate(float dt) override;
	void OnDraw() override;
};