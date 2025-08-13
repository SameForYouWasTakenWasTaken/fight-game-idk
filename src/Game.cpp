#include <typeinfo>
#include "Game.h"
#include "NPCs/Player.h"

Game::Game(int height, int width, const char* title)
	: m_Height(height), m_Width(width), m_Title(title)
{}

/**
 * @brief Initializes the window and runs the main game loop.
 *
 * Opens a window using the Game instance's width, height, and title, configures logging
 * and target framerate, creates initial game entities (player and enemy) and stores
 * them in the game's entity list, then enters the main loop. Each frame it calculates
 * delta time, calls update(dt), clears the screen, calls draw() to render entities,
 * and continues until the window is closed. Closes the window on exit.
 */
void Game::run()
{
	InitWindow(m_Width, m_Height, m_Title);
	SetTraceLogLevel(TraceLogLevel::LOG_ERROR);

	std::shared_ptr<Player> player = std::make_shared<Player>();
	std::shared_ptr<Entity> enemy = std::make_shared<Entity>("resources/Player/idle.png", "Enemy", 100.f);

	m_Entities.push_back(player);
	m_Entities.push_back(enemy);
	enemy->GetPosition() = { 500, 0 };
	SetTargetFPS(144);
	while (!WindowShouldClose())
	{
		float dt = GetFrameTime();
		// Update
		update(dt);
		// Draw stuff
		BeginDrawing();
		ClearBackground(RED);

		draw(); // Draw all essentials
		
		EndDrawing();
		
	}
	
	CloseWindow();
}

/**
 * @brief Update all game entities for the current frame.
 *
 * Processes each entity in m_Entities: advances simulation by dt, runs collision
 * checks, prunes bullets from any Player that have collided, and finally
 * removes entities flagged as not alive.
 *
 * @param dt Frame delta time in seconds used to advance entity state.
 *
 * Notes:
 * - Null entries in m_Entities are ignored.
 * - Player detection is performed via dynamic_cast; when a Player is found,
 *   its m_Bullets vector is filtered to remove bullets whose CheckCollision
 *   returns true.
 * - Entities that return false from IsAlive() after updates are removed from
 *   m_Entities at the end of the call.
 */
void Game::update(float dt)
{
	for (const auto& entity : m_Entities)
	{
		if (!entity) continue;

		entity->Update(dt);
		entity->CheckCollision(m_Entities);

		if (auto player = dynamic_cast<Player*>(entity.get()))
		{
			player->m_Bullets.erase(
				std::remove_if(player->m_Bullets.begin(), player->m_Bullets.end(),
					[&](auto& bullet) {
						return bullet->CheckCollision(m_Entities);
					}),
				player->m_Bullets.end()
			);
		}
	}

	m_Entities.erase(
		std::remove_if(m_Entities.begin(), m_Entities.end(),
			[](const std::shared_ptr<Entity>& e) {
				return !e->IsAlive();
			}),
		m_Entities.end()
	);
}


/**
 * @brief Render all game entities.
 *
 * Iterates over the current entity list and invokes each entity's Draw() method
 * to render it to the active frame. Entities are drawn in the order they
 * appear in m_Entities.
 */
void Game::draw()
{
	for (const auto& entity : m_Entities)
	{
		entity->Draw();
	}
}